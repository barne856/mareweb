#include "mareweb/renderer.hpp"
#include <SDL2/SDL_video.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace mareweb {

renderer::renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, renderer_properties properties)
    : m_device(device), m_surface(std::move(surface)), m_window(window), m_properties(std::move(properties)),
      m_clear_color({0.0F, 0.0F, 0.0F, 1.0F}) {
  wgpu::SurfaceCapabilities capabilities{};
  m_surface.GetCapabilities(m_device.GetAdapter(), &capabilities);
  // m_surface_format = *capabilities.formats;
  m_surface_format = wgpu::TextureFormat::BGRA8Unorm;

  configure_surface();

  if (m_properties.sample_count > 1) {
    try {
      create_msaa_texture();
    } catch (const std::exception &e) {
      std::cout << "MSAA not supported: " << e.what() << ". Falling back to no MSAA." << std::endl;
      m_properties.sample_count = 1;
    }
  }

  attach_system<renderer_render_system>();
  attach_system<renderer_physics_system>();
  attach_system<renderer_controls_system>();
}

renderer::~renderer() {
  if (m_window != nullptr) {
    SDL_DestroyWindow(m_window);
  }
}

void renderer::resize(uint32_t new_width, uint32_t new_height) {
  m_properties.width = new_width;
  m_properties.height = new_height;
  configure_surface();
  if (m_properties.sample_count > 1) {
    create_msaa_texture(); // Recreate MSAA texture with new size
  }
}

void renderer::present() { m_surface.Present(); }

auto renderer::create_mesh(wgpu::PrimitiveTopology topology, const std::vector<float> &vertices,
                           const std::vector<uint32_t> &indices) -> std::unique_ptr<mesh> {
  return std::make_unique<mesh>(m_device, topology, vertices, indices);
}

std::unique_ptr<material> renderer::create_material(const std::string &vertex_shader_source,
                                                    const std::string &fragment_shader_source,
                                                    const std::vector<uniform_info> &uniform_infos) {
  return std::make_unique<material>(m_device, vertex_shader_source, fragment_shader_source, m_surface_format,
                                    m_properties.sample_count, uniform_infos);
}

void renderer::set_fullscreen(bool fullscreen) {
  if (fullscreen != m_properties.fullscreen) {
    m_properties.fullscreen = fullscreen;
    if (fullscreen) {
      // Get the display index of the window
      int display_index = SDL_GetWindowDisplayIndex(m_window);
      if (display_index < 0) {
        throw std::runtime_error("Failed to get window display index");
      }

      // Get the display mode of the display
      SDL_DisplayMode display_mode;
      if (SDL_GetCurrentDisplayMode(display_index, &display_mode) != 0) {
        throw std::runtime_error("Failed to get current display mode");
      }

      // Set fullscreen
      if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
        throw std::runtime_error("Failed to set fullscreen mode");
      }

      // Update width and height to match the display
      m_properties.width = static_cast<uint32_t>(display_mode.w);
      m_properties.height = static_cast<uint32_t>(display_mode.h);
    } else {
      // Exit fullscreen mode
      if (SDL_SetWindowFullscreen(m_window, 0) != 0) {
        throw std::runtime_error("Failed to exit fullscreen mode");
      }

      // Restore the original window size
      SDL_SetWindowSize(m_window, m_properties.width, m_properties.height);
    }

    // Resize the renderer to match the new dimensions
    resize(m_properties.width, m_properties.height);
  }
}

void renderer::set_present_mode(wgpu::PresentMode present_mode) {
  if (present_mode != m_properties.present_mode) {
    m_properties.present_mode = present_mode;
    configure_surface(); // Reconfigure surface to apply present mode change
  }
}

void renderer::begin_frame() {
  wgpu::SurfaceTexture surface_texture{};
  m_surface.GetCurrentTexture(&surface_texture);
  if (!surface_texture.texture) {
    throw std::runtime_error("Failed to get current surface texture");
  }
  m_current_texture_view = surface_texture.texture.CreateView();
  if (!m_current_texture_view) {
    throw std::runtime_error("Failed to create view for surface texture");
  }

  m_command_encoder = m_device.CreateCommandEncoder();
  if (!m_command_encoder) {
    throw std::runtime_error("Failed to create command encoder");
  }

  wgpu::RenderPassColorAttachment color_attachment{};
  if (m_properties.sample_count > 1) {
    if (!m_msaa_texture_view) {
      throw std::runtime_error("MSAA texture view is null");
    }
    color_attachment.view = m_msaa_texture_view;
    color_attachment.resolveTarget = m_current_texture_view;
    color_attachment.storeOp = wgpu::StoreOp::Discard;
  } else {
    color_attachment.view = m_current_texture_view;
    color_attachment.resolveTarget = nullptr;
    color_attachment.storeOp = wgpu::StoreOp::Store;
  }
  color_attachment.loadOp = wgpu::LoadOp::Clear;
  color_attachment.clearValue = m_clear_color;

  wgpu::RenderPassDescriptor render_pass_descriptor{};
  render_pass_descriptor.colorAttachmentCount = 1;
  render_pass_descriptor.colorAttachments = &color_attachment;

  m_render_pass = m_command_encoder.BeginRenderPass(&render_pass_descriptor);
  if (!m_render_pass) {
    std::stringstream ss;
    ss << "Failed to begin render pass. "
       << "Sample count: " << m_properties.sample_count
       << ", MSAA view valid: " << (m_msaa_texture_view ? "true" : "false")
       << ", Surface view valid: " << (m_current_texture_view ? "true" : "false");
    throw std::runtime_error(ss.str());
  }
}

void renderer::end_frame() {
  m_render_pass.End();
  wgpu::CommandBuffer commands = m_command_encoder.Finish();
  m_device.GetQueue().Submit(1, &commands);
  m_surface.Present();
}

void renderer::draw_mesh(const mesh &mesh, material &material) {
  material.bind(m_render_pass, mesh.get_primitive_topology());
  mesh.draw(m_render_pass);
}

void renderer::update_model_view_projection(const transform &model_transform, const camera &cam) {
  if (!m_mvp_buffer) {
    m_mvp_buffer = std::make_shared<uniform_buffer>(m_device, sizeof(mat4), wgpu::ShaderStage::Vertex);
  }

  mat4 model_matrix = model_transform.get_transformation_matrix();
  mat4 view_matrix = cam.get_view_matrix();
  mat4 projection_matrix = cam.get_projection_matrix();
  mat4 mvp = projection_matrix * view_matrix * model_matrix;

  m_mvp_buffer->update(&mvp, sizeof(mat4));
}

void renderer::configure_surface() {
  wgpu::SurfaceConfiguration config{};
  config.device = m_device;
  config.format = m_surface_format;
  config.usage = wgpu::TextureUsage::RenderAttachment;
  config.alphaMode = wgpu::CompositeAlphaMode::Auto;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.width = m_properties.width;
  config.height = m_properties.height;
  config.presentMode = m_properties.present_mode;

  m_surface.Configure(&config);
}

void renderer::create_msaa_texture() {
  if (m_properties.sample_count <= 1) {
    return;
  }

  wgpu::TextureDescriptor texture_desc{};
  texture_desc.size.width = m_properties.width;
  texture_desc.size.height = m_properties.height;
  texture_desc.size.depthOrArrayLayers = 1;
  texture_desc.sampleCount = m_properties.sample_count;
  texture_desc.format = m_surface_format;
  texture_desc.mipLevelCount = 1;
  texture_desc.usage = wgpu::TextureUsage::RenderAttachment;

  m_msaa_texture = m_device.CreateTexture(&texture_desc);
  if (!m_msaa_texture) {
    throw std::runtime_error("Failed to create MSAA texture");
  }
  m_msaa_texture_view = m_msaa_texture.CreateView();
  if (!m_msaa_texture_view) {
    throw std::runtime_error("Failed to create MSAA texture view");
  }
}

} // namespace mareweb