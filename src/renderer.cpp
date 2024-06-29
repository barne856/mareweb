#include "mareweb/renderer.hpp"
#include <stdexcept>

namespace mareweb {

Renderer::Renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, uint32_t width, uint32_t height)
    : m_device(device), m_surface(surface), m_window(window), m_width(width), m_height(height) {
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
  m_width = newWidth;
  m_height = newHeight;
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

void Renderer::drawMesh(const Mesh& mesh, const Material& material) {
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
  config.width = m_width;
  config.height = m_height;
  config.presentMode = wgpu::PresentMode::Fifo;

  m_surface.Configure(&config);
}

} // namespace mareweb