#include "mareweb/application.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/materials/textured_material.hpp"
#include "mareweb/meshes/primitive_mesh.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/quantity_types.hpp"
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
    ent.update_transforms(dt);
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
    set_position(vec3_t<length>{length(0.0F), length(-2.0F), length(0.0F)});
    face_towards(vec3_t<length>{length(0.0F), length(0.0F), length(0.0F)}, vec3{0.0F, 0.0F, -1.0F});
    m_basic_entity = create_object<basic_entity>(this);
    float aspect_ratio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
    set_aspect_ratio(aspect_ratio);
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
    // Create a square mesh with proper texture coordinates
    // mesh = scene->create_mesh<mareweb::square_mesh>(length(1.0));
    mesh = scene->create_mesh<mareweb::sphere_mesh>(length(0.5), 32, 32);

    // Create a textured material (provide path to your texture)
    material = scene->create_material<mareweb::textured_material>("assets/2k_earth_daymap.jpg");

    // Set initial light direction
    vec3 light_direction{1.f, -1.f, 0.f};
    material->update_light_direction(light_direction);

    attach_system<render_mesh>();
  }

  void update_transforms(const squint::duration &dt) {
    // Rotate the entity
    auto freq = frequency(1);
    rotate(vec3{0, 0, 1}, -units::degrees(25) * dt * freq);

    // Get the updated MVP matrix from the renderer
    mat4 mvp = scene->get_mvp_matrix(*this);
    mat3 normal_mat = get_normal_matrix();

    // Update the transforms
    material->update_mvp(mvp);
    material->update_normal_matrix(normal_mat);
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::textured_material> material;
  main_scene *scene;
};

int main() {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 1920,
                                        .height = 1080,
                                        .title = "Basic Texture",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_scene>(props);
  app.run();

  return 0;
}