#include "mareweb/renderer.hpp"
#include <SDL3/SDL_video.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace mareweb {

renderer::renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                   const renderer_properties &properties)
    : m_device(device), m_surface(surface), m_window(window), m_properties(properties) {
  wgpu::SurfaceCapabilities capabilities{};
  m_surface.GetCapabilities(m_device.GetAdapter(), &capabilities);
  m_surface_format = *capabilities.formats;

  configure_surface();

  if (m_properties.sample_count > 1) {
    try {
      create_msaa_texture();
    } catch (const std::exception &e) {
      std::cout << "MSAA not supported: " << e.what() << ". Falling back to no MSAA." << std::endl;
      m_properties.sample_count = 1;
    }
  }
}

renderer::~renderer() {
  if (m_window) {
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

std::unique_ptr<mesh> renderer::create_mesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices) {
  return std::make_unique<mesh>(m_device, vertices, indices);
}

std::unique_ptr<material> renderer::create_material(const std::string &vertex_shader_source,
                                                    const std::string &fragment_shader_source) {
  return std::make_unique<material>(m_device, vertex_shader_source, fragment_shader_source, m_surface_format,
                                    m_properties.sample_count);
}

void renderer::set_fullscreen(bool fullscreen) {
  if (fullscreen != m_properties.fullscreen) {
    m_properties.fullscreen = fullscreen;

    if (fullscreen) {
      // Get the display index of the window
      int count_displays;
      SDL_DisplayID *displays = SDL_GetDisplays(&count_displays);
      if (!displays) {
        // Handle error, perhaps throw an exception
        throw std::runtime_error("Failed to get window display index");
      }

      // Get the display mode of the display
      const SDL_DisplayMode *display_mode = SDL_GetCurrentDisplayMode(displays[0]);
      if (!display_mode) {
        SDL_free(displays);
        // Handle error, perhaps throw an exception
        throw std::runtime_error("Failed to get current display mode");
      }

      // Set fullscreen
      if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN) != 0) {
        // Handle error, perhaps throw an exception
        SDL_free(displays);
        throw std::runtime_error("Failed to set fullscreen mode");
      }

      // Update width and height to match the display
      m_properties.width = static_cast<uint32_t>(display_mode->w);
      m_properties.height = static_cast<uint32_t>(display_mode->h);
      SDL_free(displays);
    } else {
      // Exit fullscreen mode
      if (SDL_SetWindowFullscreen(m_window, 0) != 0) {
        // Handle error, perhaps throw an exception
        throw std::runtime_error("Failed to exit fullscreen mode");
      }

      // Restore original width and height (you might want to store these separately)
      // For now, we'll just set a default size
      m_properties.width = 800;  // or whatever default you prefer
      m_properties.height = 600; // or whatever default you prefer
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

void renderer::draw_mesh(const mesh &mesh, const material &material) {
  material.bind(m_render_pass);
  mesh.draw(m_render_pass);
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