#include "mareweb/application.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entities/arrow_2d.hpp"
#include "mareweb/entities/text.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/quantity_types.hpp"
#include "webgpu/webgpu_cpp.h"
#include <memory>
#include <vector>

using namespace squint;

class basic_entity;

class main_scene : public mareweb::scene {
public:
  main_scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
             const mareweb::renderer_properties &properties,
             mareweb::projection_type type = mareweb::projection_type::perspective)
      : scene(device, surface, window, properties, type) {
    set_clear_color({0.05F, 0.05F, 0.05F, 1.0F});
    set_position(vec3_t<length>{length(0.0F), length(0.0F), length(2.0F)});
    m_text = create_object<mareweb::text>(this, "HELLO,\nWORLD", 0.05F, 0.1F);
    // m_text->set_scale(vec3{0.7F, 0.7F, 0.7F});
    m_text->set_center({0.0F, 0.0F});
    float aspect_ratio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
    set_aspect_ratio(aspect_ratio);
  }

  auto on_resize(const mareweb::window_resize_event &event) -> bool override {
    auto aspect_ratio = static_cast<float>(event.width) / static_cast<float>(event.height);
    set_aspect_ratio(aspect_ratio);
    return true;
  }

  void render(const squint::duration &dt) override {
    scene::render(dt);
    // m_text->rotate(vec3{0.0F, 1.0F, 1.0F}, units::degrees(1.0F));
    // blinking exclamation point
    static squint::duration elapsed_time;
    elapsed_time += dt;
    if (elapsed_time >= squint::duration(0.75F)) {
      elapsed_time = squint::duration(0.0F);
      if (m_text->get_text().back() == '!') {
        m_text->set_text("HELLO,\nWORLD");
      } else {
        m_text->set_text("HELLO,\nWORLD!");
      }
    }
  }

private:
  mareweb::text *m_text;
};

int main() {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 800,
                                        .height = 600,
                                        .title = "Basic Mesh",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_scene>(props);

  app.run();

  return 0;
}