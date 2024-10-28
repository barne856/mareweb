#include "mareweb/application.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/materials/flat_color_material.hpp"
#include "mareweb/meshes/primitive_mesh.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "webgpu/webgpu_cpp.h"
#include <vector>

using namespace squint;

class basic_entity;

template <typename T>
concept renderable_mesh = requires(T t) {
  { t.mesh.get() } -> std::convertible_to<mareweb::mesh *>;
  { t.material.get() } -> std::convertible_to<mareweb::material *>;
};

template <typename T>
  requires renderable_mesh<T>
class render_mesh : public mareweb::render_system<T> {
public:
  void render(const squint::duration &dt, T &ent) override {
    ent.update_mvp(dt);
    ent.scene->draw_mesh(*ent.mesh.get(), *ent.material.get());
  }
};

class main_scene : public mareweb::scene {
public:
  main_scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
             const mareweb::renderer_properties &properties,
             mareweb::projection_type type = mareweb::projection_type::perspective)
      : scene(device, surface, window, properties, type) {
    set_clear_color({0.05F, 0.05F, 0.05F, 1.0F});
    set_position(vec3_t<length>{length(0.0F), length(0.0F), length(2.0F)});
    m_basic_entity = create_object<basic_entity>(this);
    float aspect_ratio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
    set_aspect_ratio(aspect_ratio);
  }

  auto on_resize(const mareweb::window_resize_event &event) -> bool override {
    auto aspect_ratio = static_cast<float>(event.width) / static_cast<float>(event.height);
    set_aspect_ratio(aspect_ratio);
    return true;
  }

  mat4 get_mvp_matrix(const mareweb::transform &model_transform) {
    mat4 model_matrix = model_transform.get_transformation_matrix();
    mat4 view_projection_matrix = get_view_projection_matrix();
    return view_projection_matrix * model_matrix;
  }

private:
  basic_entity *m_basic_entity;
};

class basic_entity : public mareweb::entity<basic_entity>, public mareweb::transform {
public:
  basic_entity(main_scene *scene) : scene(scene) {
    // -- try out different meshes --
    // mesh = scene->create_mesh<mareweb::triangle_mesh>(
    //     squint::vec3_t<length>{length(0), length(0.5), length(0.0)},     // Top vertex
    //     squint::vec3_t<length>{length(-0.5), length(-0.5), length(0.0)}, // Bottom-left vertex
    //     squint::vec3_t<length>{length(0.5), length(-0.5), length(0.0)}   // Bottom-right vertex
    // );
    // mesh = scene->create_mesh<mareweb::circle_mesh>(length(0.5), 50);
    mesh = scene->create_mesh<mareweb::sphere_mesh>(length(0.4), 3);
    // mesh = scene->create_mesh<mareweb::sphere_mesh>(length(0.5), 10, 10);
    // mesh = scene->create_mesh<mareweb::cube_mesh>(length(0.5));
    // mesh = scene->create_mesh<mareweb::square_mesh>(length(0.5));
    vec4 color{1.F, 1.F, 0.F, 0.F};
    vec3 light_direction{1.f, 1.f, -1.f};
    material = scene->create_material<mareweb::flat_color_material>(color);
    material->update_light_direction(light_direction);
    attach_system<render_mesh>();
  }

  void update_mvp(const squint::duration &dt) {
    // Get the updated MVP matrix from the renderer
    mat4 mvp = scene->get_mvp_matrix(*this);

    // Update the uniform buffer
    material->update_mvp(mvp);
    material->update_normal_matrix(get_normal_matrix());

    // Update color for rainbow effect
    static float total_time = 0.0f;
    total_time += dt.value();

    // Use a simple HSV to RGB conversion for rainbow effect
    float hue = std::fmod(total_time * 0.2f, 1.0f); // Cycle through hues
    float saturation = 1.0f;
    float value = 1.0f;

    float c = value * saturation;
    float x = c * (1 - std::abs(std::fmod(hue * 6, 2) - 1));
    float m = value - c;

    float r, g, b;
    if (hue < 1.0f / 6.0f) {
      r = c;
      g = x;
      b = 0;
    } else if (hue < 2.0f / 6.0f) {
      r = x;
      g = c;
      b = 0;
    } else if (hue < 3.0f / 6.0f) {
      r = 0;
      g = c;
      b = x;
    } else if (hue < 4.0f / 6.0f) {
      r = 0;
      g = x;
      b = c;
    } else if (hue < 5.0f / 6.0f) {
      r = x;
      g = 0;
      b = c;
    } else {
      r = c;
      g = 0;
      b = x;
    }

    vec4 color = {r + m, g + m, b + m, 1.0f};

    // Update the color uniform buffer
    material->update_uniform(1, &color);
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::flat_color_material> material;
  std::shared_ptr<mareweb::uniform_buffer> mvp_buffer;
  main_scene *scene;
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