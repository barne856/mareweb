#include "mareweb/application.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entities/camera.hpp"
#include "mareweb/meshes/primitive_mesh.hpp"
#include "mareweb/materials/flat_color_material.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "webgpu/webgpu_cpp.h"

using namespace squint;

class cube;
template <typename T>
class render_cube;

class main_scene : public mareweb::scene {
public:
  main_scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
             const mareweb::renderer_properties &properties)
      : scene(device, surface, window, properties, mareweb::projection_type::perspective) {
    set_clear_color({0.1F, 0.1F, 0.1F, 1.0F});
    get_camera()->set_position(vec3_t<units::length>{units::length(0.0F), units::length(0.0F), units::length(5.0F)});
    m_cube = create_object<cube>(this, device);
  }

  mat4 get_mvp_matrix(const mareweb::transform &model_transform) {
    mat4 model_matrix = model_transform.get_transformation_matrix();
    mat4 view_projection_matrix = get_camera()->get_view_projection_matrix();
    return view_projection_matrix * model_matrix;
  }

private:
  cube *m_cube;
};

class cube : public mareweb::entity<cube>, public mareweb::transform {
public:
  cube(main_scene *scene, wgpu::Device &device) : scene(scene) {
    mesh = scene->create_mesh<mareweb::cube_mesh>(units::length(1.0F));  // 1 meter cube
    vec4 blue_color{0.0F, 0.0F, 1.0F, 1.0F};
    material = scene->create_material<mareweb::flat_color_material>(blue_color);
    attach_system<render_cube>();
  }

  void update_mvp(const units::time &dt) {
    // Rotate the cube
    units::frequency f(0.5f);
    rotate(vec3{1, 1, 1}, f * dt);

    // Get the updated MVP matrix from the renderer
    mat4 mvp = scene->get_mvp_matrix(*this);

    // Update the MVP in the material
    material->update_mvp(mvp);
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::flat_color_material> material;
  main_scene *scene;
};

template <typename T>
class render_cube : public mareweb::render_system<T> {
public:
  void render(const units::time &dt, T &ent) override {
    ent.update_mvp(dt);
    ent.scene->draw_mesh(*ent.mesh.get(), *ent.material.get());
  }
};

auto main() -> int {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {
    .width = 800,
    .height = 600,
    .title = "Rotating Blue Cube",
    .fullscreen = false,
    .resizable = true,
    .present_mode = wgpu::PresentMode::Fifo,
    .sample_count = 4
  };

  app.create_renderer<main_scene>(props);

  app.run();

  return 0;
}