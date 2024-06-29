#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <cmath>
#include <iostream>

class BasicRenderer : public mareweb::Renderer {
public:
  using mareweb::Renderer::Renderer; // Inherit constructor

  void render() override {
    // set clear color based on time
    m_clearColor = {std::abs(std::sin(m_time)), std::abs(std::cos(m_time)), 0.0f, 1.0f};
    setClearColor(m_clearColor);
    m_time += 0.01f;
  }

private:
  float m_time = 0.0f;
};

int main() {
  try {
    mareweb::Application &app = mareweb::Application::getInstance();
    app.initialize();

    mareweb::RendererProperties props1 = {.width = 800,
                                          .height = 600,
                                          .title = "Basic Renderer 1",
                                          .fullscreen = false,
                                          .resizable = true,
                                          .presentMode = wgpu::PresentMode::Fifo,
                                          .sampleCount = 4};

    mareweb::RendererProperties props2 = {.width = 400,
                                          .height = 300,
                                          .title = "Basic Renderer 2",
                                          .fullscreen = false,
                                          .resizable = false,
                                          .presentMode = wgpu::PresentMode::Immediate,
                                          .sampleCount = 1};

    app.createRenderer<BasicRenderer>(props1);
    app.createRenderer<BasicRenderer>(props2);

    app.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}