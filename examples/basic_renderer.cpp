#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <cmath>
#include <iostream>

using namespace squint;

class basic_renderer : public mareweb::renderer {
public:
  using mareweb::renderer::renderer; // Inherit constructor

  void render(const units::time& dt) override {
    begin_frame();
    // set clear color based on time
    m_time += dt;
    set_clear_color({std::abs(std::sin(m_time.value())), std::abs(std::cos(m_time.value())), 0.0f, 1.0f});
    end_frame();
  }

private:
  units::time m_time{};
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