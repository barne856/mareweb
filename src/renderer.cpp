#include "mareweb/renderer.hpp"
#include "mareweb/material.hpp"
#include <SDL2/SDL_video.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace mareweb {

renderer::renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, renderer_properties properties)
    : m_device(device), m_surface(std::move(surface)), m_window(window), m_properties(std::move(properties)),
      m_clear_color({0.0F, 0.0F, 0.0F, 1.0F}) {
  // wgpu::SurfaceCapabilities capabilities{};
  // m_surface.GetCapabilities(m_device.GetAdapter(), &capabilities);
  // m_surface_format = *capabilities.formats;
  m_surface_format = wgpu::TextureFormat::BGRA8Unorm;

  configure_surface();
  create_depth_texture();

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
  create_depth_texture();
  if (m_properties.sample_count > 1) {
    create_msaa_texture(); // Recreate MSAA texture with new size
  }
}

void renderer::present() {
#ifndef __EMSCRIPTEN__
  m_surface.Present();
#endif
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

  // Setup depth attachment
  wgpu::RenderPassDepthStencilAttachment depth_attachment{};
  depth_attachment.view = m_depth_texture_view;
  depth_attachment.depthClearValue = 1.0f;
  depth_attachment.depthLoadOp = wgpu::LoadOp::Clear;
  depth_attachment.depthStoreOp = wgpu::StoreOp::Store;
  depth_attachment.stencilLoadOp = wgpu::LoadOp::Undefined;
  depth_attachment.stencilStoreOp = wgpu::StoreOp::Undefined;
  depth_attachment.depthReadOnly = false;

  wgpu::RenderPassDescriptor render_pass_descriptor{};
  render_pass_descriptor.colorAttachmentCount = 1;
  render_pass_descriptor.colorAttachments = &color_attachment;
  render_pass_descriptor.depthStencilAttachment = &depth_attachment;

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
#ifndef __EMSCRIPTEN__
  m_surface.Present();
#endif
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

void renderer::create_depth_texture() {
  wgpu::TextureDescriptor depth_tex_desc{};
  depth_tex_desc.dimension = wgpu::TextureDimension::e2D;
  depth_tex_desc.format = wgpu::TextureFormat::Depth24Plus;
  depth_tex_desc.mipLevelCount = 1;
  depth_tex_desc.sampleCount = m_properties.sample_count;
  depth_tex_desc.size = {m_properties.width, m_properties.height, 1};
  depth_tex_desc.usage = wgpu::TextureUsage::RenderAttachment;
  depth_tex_desc.viewFormats = nullptr;
  depth_tex_desc.viewFormatCount = 0;

  m_depth_texture = m_device.CreateTexture(&depth_tex_desc);
  m_depth_texture_view = m_depth_texture.CreateView();
}

} // namespace mareweb