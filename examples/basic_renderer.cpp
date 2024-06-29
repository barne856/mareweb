#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <cmath>
#include <iostream>

class BasicRenderer : public mareweb::Renderer {
public:
  using mareweb::Renderer::Renderer; // Inherit constructor

  void render() override {
    wgpu::SurfaceTexture surfaceTexture{};
    m_surface.GetCurrentTexture(&surfaceTexture);
    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder();

    const wgpu::Color clearColor{std::sin(m_time + 1.0F), std::cos(m_time), std::sin(m_time), 1.0F};
    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = view;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = clearColor;

    wgpu::RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    m_device.GetQueue().Submit(1, &commands);

    m_time += 0.01f;
  }

private:
  float m_time = 0.0f;
};

int main() {
  try {
    mareweb::Application &app = mareweb::Application::getInstance();
    app.initialize();

    app.createRenderer<BasicRenderer>(800, 600);
    app.createRenderer<BasicRenderer>(400, 300);

    app.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}