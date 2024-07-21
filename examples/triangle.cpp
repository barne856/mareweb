#include "mareweb/application.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entities/camera.hpp"
#include "mareweb/renderer.hpp"
#include "mareweb/scene.hpp"
#include "squint/quantity.hpp"
#include <vector>

using namespace squint;

class triangle;

template <typename T>
concept renderable_mesh = requires(T t) {
  { t.mesh.get() } -> std::convertible_to<mareweb::mesh *>;
  { t.material.get() } -> std::convertible_to<mareweb::material *>;
};

template <typename T>
  requires renderable_mesh<T>
class render_mesh : public mareweb::render_system<T> {
public:
  void render(const units::time &dt, T &ent) override {
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
    get_camera()->set_position(vec3_t<units::length>{units::length(0.0F), units::length(0.0F), units::length(2.0F)});
    m_triangle = create_object<triangle>(this, device);
  }

  mat4 get_mvp_matrix(const mareweb::transform &model_transform) {
    mat4 model_matrix = model_transform.get_transformation_matrix();
    mat4 view_projection_matrix = get_camera()->get_view_projection_matrix();
    return view_projection_matrix * model_matrix;
  }

private:
  triangle *m_triangle;
};

class triangle : public mareweb::entity<triangle>, public mareweb::transform {
public:
  triangle(main_scene *scene, wgpu::Device &device) : scene(scene) {
    std::vector<float> vertices = {0.0F, 0.5F, 0.0F, -0.5F, -0.5F, 0.0F, 0.5F, -0.5F, 0.0F};

    const char *vertex_shader_source = R"(
        @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;

            @vertex
            fn main(@location(0) position: vec3<f32>) -> @builtin(position) vec4<f32> {
                return mvp * vec4<f32>(position, 1.0);
            }
        )";

    const char *fragment_shader_source = R"(
            @fragment
            fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(1.0, 0.1, 0.05, 1.0);
            }
        )";

    mesh = scene->create_mesh(vertices);
    material = scene->create_material(vertex_shader_source, fragment_shader_source);

    // Create and add uniform buffer
    mvp_buffer = std::make_shared<mareweb::uniform_buffer>(device, sizeof(mat4), wgpu::ShaderStage::Vertex);
    material->add_uniform_buffer(0, mvp_buffer);

    attach_system<render_mesh>();
  }

  void update_mvp(const units::time &dt) {
    // Rotate the triangle
    rotate(vec3{0, 1, 0}, 1.0f * dt.value());

    // Get the updated MVP matrix from the renderer
    mat4 mvp = scene->get_mvp_matrix(*this);

    // Update the uniform buffer
    mvp_buffer->update(&mvp, sizeof(mat4));
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::material> material;
  std::shared_ptr<mareweb::uniform_buffer> mvp_buffer;
  main_scene *scene;
};

auto main() -> int {
  mareweb::application &app = mareweb::application::get_instance();
  app.initialize();

  mareweb::renderer_properties props = {.width = 800,
                                        .height = 600,
                                        .title = "Triangle with MVP",
                                        .fullscreen = false,
                                        .resizable = true,
                                        .present_mode = wgpu::PresentMode::Fifo,
                                        .sample_count = 4};

  app.create_renderer<main_scene>(props);

  app.run();

  return 0;
}