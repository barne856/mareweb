#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <cmath>
#include <iostream>

class BasicRenderer : public mareweb::Renderer {
public:
  using mareweb::Renderer::Renderer; // Inherit constructor

  void render() override {
    // Clear the screen
    wgpu::Color clearColor = {0.0f, std::sin(m_time), 0.0f, 1.0f};
    setClearColor(clearColor);

    m_time += 0.01f*static_cast<float>(m_width)*0.01f;
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