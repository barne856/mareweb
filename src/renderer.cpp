#include "mareweb/renderer.hpp"
#include <SDL3/SDL_video.h>
#include <stdexcept>

namespace mareweb {

Renderer::Renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                   const RendererProperties &properties)
    : m_device(device), m_surface(surface), m_window(window), m_properties(properties) {
  wgpu::SurfaceCapabilities capabilities{};
  m_surface.GetCapabilities(m_device.GetAdapter(), &capabilities);
  m_surfaceFormat = *capabilities.formats;
  configureSurface();
}

Renderer::~Renderer() {
  if (m_window) {
    SDL_DestroyWindow(m_window);
  }
}

void Renderer::resize(uint32_t newWidth, uint32_t newHeight) {
  m_properties.width = newWidth;
  m_properties.height = newHeight;
  configureSurface();
}

void Renderer::present() { m_surface.Present(); }

std::unique_ptr<Mesh> Renderer::createMesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices) {
  return std::make_unique<Mesh>(m_device, vertices, indices);
}

std::unique_ptr<Material> Renderer::createMaterial(const std::string &vertexShaderSource,
                                                   const std::string &fragmentShaderSource) {
  return std::make_unique<Material>(m_device, vertexShaderSource, fragmentShaderSource, m_surfaceFormat);
}

void Renderer::setFullscreen(bool fullscreen) {
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

void Renderer::setPresentMode(wgpu::PresentMode presentMode) {
  if (presentMode != m_properties.presentMode) {
    m_properties.presentMode = presentMode;
    configureSurface(); // Reconfigure surface to apply present mode change
  }
}

void Renderer::beginFrame() {
  wgpu::SurfaceTexture surfaceTexture{};
  m_surface.GetCurrentTexture(&surfaceTexture);
  m_currentTextureView = surfaceTexture.texture.CreateView();

  m_commandEncoder = m_device.CreateCommandEncoder();

  wgpu::RenderPassColorAttachment colorAttachment{};
  colorAttachment.view = m_currentTextureView;
  colorAttachment.loadOp = wgpu::LoadOp::Clear;
  colorAttachment.storeOp = wgpu::StoreOp::Store;
  colorAttachment.clearValue = m_clearColor;

  wgpu::RenderPassDescriptor renderPassDescriptor{};
  renderPassDescriptor.colorAttachmentCount = 1;
  renderPassDescriptor.colorAttachments = &colorAttachment;

  m_renderPass = m_commandEncoder.BeginRenderPass(&renderPassDescriptor);
}

void Renderer::endFrame() {
  m_renderPass.End();
  wgpu::CommandBuffer commands = m_commandEncoder.Finish();
  m_device.GetQueue().Submit(1, &commands);
  m_surface.Present();
}

void Renderer::drawMesh(const Mesh &mesh, const Material &material) {
  material.bind(m_renderPass);
  mesh.draw(m_renderPass);
}

void Renderer::configureSurface() {
  wgpu::SurfaceConfiguration config{};
  config.device = m_device;
  config.format = m_surfaceFormat;
  config.usage = wgpu::TextureUsage::RenderAttachment;
  config.alphaMode = wgpu::CompositeAlphaMode::Auto;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.width = m_properties.width;
  config.height = m_properties.height;
  config.presentMode = m_properties.presentMode;

  m_surface.Configure(&config);
}

} // namespace mareweb