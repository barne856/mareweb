#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <cmath>
#include <iostream>

class basic_renderer : public mareweb::renderer {
public:
  using mareweb::renderer::renderer; // Inherit constructor

  void render(float dt) override {
    // set clear color based on time
    m_clear_color = {std::abs(std::sin(m_time)), std::abs(std::cos(m_time)), 0.0f, 1.0f};
    set_clear_color(m_clear_color);
    m_time += 0.01f;
  }

private:
  float m_time = 0.0f;
};

int main() {
  try {
    mareweb::application &app = mareweb::application::get_instance();
    app.initialize();

    mareweb::renderer_properties props1 = {.width = 800,
                                           .height = 600,
                                           .title = "Basic Renderer 1",
                                           .fullscreen = false,
                                           .resizable = true,
                                           .present_mode = wgpu::PresentMode::Fifo,
                                           .sample_count = 4};

    mareweb::renderer_properties props2 = {.width = 400,
                                           .height = 300,
                                           .title = "Basic Renderer 2",
                                           .fullscreen = false,
                                           .resizable = false,
                                           .present_mode = wgpu::PresentMode::Immediate,
                                           .sample_count = 1};

    app.create_renderer<basic_renderer>(props1);
    app.create_renderer<basic_renderer>(props2);

    app.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}