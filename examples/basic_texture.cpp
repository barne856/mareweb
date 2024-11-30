#include "mareweb/application.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entities/renderable.hpp"
#include "mareweb/materials/textured_material.hpp"
#include "mareweb/meshes/primitive_mesh.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/quantity_types.hpp"
#include "squint/quantity/unit_types.hpp"
#include "squint/tensor/tensor_types.hpp"
#include "webgpu/webgpu_cpp.h"
#include <iostream>
#include <vector>

using namespace squint;

class basic_entity;

class main_scene : public mareweb::scene {
public:
  main_scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
             const mareweb::renderer_properties &properties,
             mareweb::projection_type type = mareweb::projection_type::perspective)
      : scene(device, surface, window, properties, type) {
    set_clear_color({0.0F, 0.0F, 0.0F, 1.0F});
    set_position(vec3_t<length>{length(0.0F), length(-2.0F), length(0.0F)});
    face_towards(vec3_t<length>{length(0.0F), length(0.0F), length(0.0F)}, vec3{0.0F, 0.0F, -1.0F});
    m_basic_entity = create_object<basic_entity>(this);
    float aspect_ratio = static_cast<float>(properties.width) / static_cast<float>(properties.height);
    set_aspect_ratio(aspect_ratio);
  }

  auto on_resize(const mareweb::window_resize_event &event) -> bool override {
    auto aspect_ratio = static_cast<float>(event.width) / static_cast<float>(event.height);
    set_aspect_ratio(aspect_ratio);
    return true;
  }

private:
  basic_entity *m_basic_entity;
};

class basic_entity : public mareweb::entity<basic_entity> {
public:
  basic_entity(main_scene *scene) : scene(scene) {
    // Create a square mesh with proper texture coordinates
    // mesh = scene->create_mesh<mareweb::square_mesh>(length(1.0));
    // mesh = scene->create_mesh<mareweb::sphere_mesh>(length(0.4), 3);
    mesh = scene->create_mesh<mareweb::sphere_mesh>(length(0.4), 32, 32);
    // mesh = scene->create_mesh<mareweb::cube_mesh>(length(0.5));
    // mesh = scene->create_mesh<mareweb::torus_mesh>(length(0.4), length(0.2), 32, 32);
    // mesh = scene->create_mesh<mareweb::cylinder_mesh>(length(0.4), length(0.5), 0, units::degrees(360), 32);
    // mesh = scene->create_mesh<mareweb::line_mesh>(0.2F);
    // mesh = scene->create_mesh<mareweb::char_mesh>("Hello, World!", 0.02F);

    // Create a textured material (provide path to your texture)
    material = scene->create_material<mareweb::textured_material>("assets/2k_earth_daymap.jpg");

    // Set initial light direction
    vec3 light_direction{-1.f, -2.f, -1.f};
    material->update_light_direction(light_direction);

    obj = create_object<mareweb::renderable>(scene, mesh.get(), material.get());
  }

  void update(const squint::duration &dt) override {
    entity::update(dt); // update attached systems
    // Rotate the entity
    auto freq = frequency(1);
    obj->rotate(vec3{0, 0, 1}, -units::degrees(25) * dt * freq);
  }

  void render(const squint::duration &dt) override {
    entity::render(dt); // render attached systems
    obj->render(dt);    // TODO, we should render children by default (update object class?)
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::textured_material> material;
  mareweb::renderable *obj = nullptr;
  main_scene *scene;
};

int main() {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 800,
                                        .height = 600,
                                        .title = "Basic Texture",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_scene>(props);
  app.run();

  return 0;
}