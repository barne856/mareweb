#include "mareweb/application.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/materials/flat_color_material.hpp"
#include "mareweb/meshes/primitive_mesh.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/unit_types.hpp"
#include "webgpu/webgpu_cpp.h"
#include <vector>

using namespace squint;

template <typename T>
concept renderable_mesh = requires(T t) {
  { t.mesh.get() } -> std::convertible_to<mareweb::mesh *>;
  { t.material.get() } -> std::convertible_to<mareweb::material *>;
};

template <typename T>
  requires renderable_mesh<T>
class render_mesh : public mareweb::render_system<T> {
public:
  void render(const squint::time &dt, T &ent) override {
    mat4 model_matrix = ent.get_transformation_matrix();
    mat4 view_projection_matrix = ent.scene->get_view_projection_matrix();
    ent.rotate(vec3{0.F, 0.F, 1.F}, dt.value());
    ent.material->update_mvp(view_projection_matrix * model_matrix);
    ent.scene->draw_mesh(*ent.mesh.get(), *ent.material.get());
  }
};

class square : public mareweb::entity<square>, public mareweb::transform {
public:
  square(mareweb::scene *scene) : scene(scene) {
    mesh = scene->create_mesh<mareweb::rectangle_mesh>(length(1), length(1));
    vec4 color{0.9F, 0.9F, 0.9F, 0.F};
    material = scene->create_material<mareweb::flat_color_material>(color);
    attach_system<render_mesh>();
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::flat_color_material> material;
  mareweb::scene *scene;
};

class main_scene : public mareweb::scene {
public:
  main_scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
             const mareweb::renderer_properties &properties,
             mareweb::projection_type type = mareweb::projection_type::perspective)
      : scene(device, surface, window, properties, type) {
    set_clear_color({0.15F, 0.15F, 0.15F, 1.0F});
    set_position(vec3_t<length>{length(0.0F), length(0.0F), length(2.0F)});
    m_square = create_object<square>(this);
    m_square_2 = create_object<square>(this);
    m_square_2->rotate(vec3{0.F, 0.F, 1.F}, units::degrees(45));
    float aspect_ratio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
    set_aspect_ratio(aspect_ratio);
  }

private:
  square *m_square;
  square *m_square_2;
};

int main() {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 800,
                                        .height = 600,
                                        .title = "Square Mesh",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_scene>(props);

  app.run();

  return 0;
}