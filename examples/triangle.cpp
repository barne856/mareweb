#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include "squint/linalg.hpp"
#include <concepts>
#include <iostream>
#include <vector>
#include "mareweb/components/transform.hpp"

template <typename T>
concept renderable_mesh = requires(T t) {
  { t.mesh.get() } -> std::convertible_to<mareweb::mesh *>;
  { t.material.get() } -> std::convertible_to<mareweb::material *>;
};

template <typename T>
requires renderable_mesh<T>
class render_mesh : public mareweb::render_system<T> {
public:
  void render(float dt, T &ent) override { ent.rend->draw_mesh(*ent.mesh.get(), *ent.material.get()); }
};

class triangle : public mareweb::entity<triangle>, public mareweb::transform {
public:
  triangle(mareweb::renderer *rend) : rend(rend) {
    std::vector<float> vertices = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};

    const char *vertex_shader_source = R"(
            @vertex
            fn main(@location(0) position: vec3<f32>) -> @builtin(position) vec4<f32> {
                return vec4<f32>(position, 1.0);
            }
        )";

    const char *fragment_shader_source = R"(
            @fragment
            fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(1.0, 0.1, 0.05, 1.0);
            }
        )";

    mesh = rend->create_mesh(vertices);
    material = rend->create_material(vertex_shader_source, fragment_shader_source);
    attach_system<render_mesh>();
  }
  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::material> material;
  mareweb::renderer *rend;
};

class main_renderer : public mareweb::renderer {
public:
  main_renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                const mareweb::renderer_properties &properties)
      : renderer(device, surface, window, properties) {
    set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    create_object<triangle>(this);
  }
};

int main() {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 800,
                                        .height = 600,
                                        .title = "Triangle",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_renderer>(props);

  app.run();

  return 0;
}