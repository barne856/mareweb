#include "mareweb/application.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entities/camera.hpp"
#include "mareweb/renderer.hpp"
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
    ent.rend->draw_mesh(*ent.mesh.get(), *ent.material.get());
  }
};

class main_renderer : public mareweb::renderer {
public:
  main_renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                const mareweb::renderer_properties &properties)
      : renderer(device, surface, window, properties) {
    set_clear_color({0.05F, 0.05F, 0.05F, 1.0F});
    m_camera = std::make_unique<mareweb::camera>(45.0f, float(properties.width) / float(properties.height),
                                                 length(0.1f), length(100.0f));
    m_camera->set_position(vec3_t<length>{length(0), length(0), length(3)});
    m_triangle = create_object<triangle>(this, device);
  }

  void update(const units::time &dt) override { renderer::update(dt); }

  mat4 get_mvp_matrix(const mareweb::transform &model_transform) {
    mat4 model_matrix = model_transform.get_transformation_matrix();
    mat4 view_matrix = m_camera->get_view_matrix();
    mat4 projection_matrix = m_camera->get_projection_matrix();
    return projection_matrix * view_matrix * model_matrix;
  }

private:
  std::unique_ptr<mareweb::camera> m_camera;
  triangle *m_triangle;
};

class triangle : public mareweb::entity<triangle>, public mareweb::transform {
public:
  triangle(main_renderer *rend, wgpu::Device &device) : rend(rend) {
    std::vector<float> vertices = {0.0F, 0.5F, 0.0F, -0.5F, -0.5F, 0.0F, 0.5F, -0.5F, 0.0F};

    const char *vertex_shader_source = R"(
    struct Uniforms {
        mvp: mat4x4<f32>,
    };
    @group(0) @binding(0) var<uniform> uniforms: Uniforms;

    struct VertexOutput {
        @builtin(position) position: vec4<f32>,
        @location(0) mvp_value: f32,
    };

    @vertex
    fn main(@location(0) position: vec3<f32>) -> VertexOutput {
        var output: VertexOutput;
        output.position = uniforms.mvp * vec4<f32>(position, 1.0);
        output.mvp_value = uniforms.mvp[1][1]; // Pass the first element of the MVP matrix
        return output;
    }
)";

    const char *fragment_shader_source = R"(
    @fragment
    fn main(@location(0) mvp_value: f32) -> @location(0) vec4<f32> {
        return vec4<f32>(1.0, mvp_value, 0.0, 1.0);
    }
)";

    mesh = rend->create_mesh(vertices);
    material = rend->create_material(vertex_shader_source, fragment_shader_source);

    // Create and add uniform buffer
    mvp_buffer = std::make_shared<mareweb::uniform_buffer>(device, sizeof(mat4), wgpu::ShaderStage::Vertex);
    material->add_uniform_buffer("mvp", mvp_buffer);

    attach_system<render_mesh>();
  }

  void update_mvp(const units::time &dt) {
    // Rotate the triangle
    rotate(vec3{0, 1, 0}, 1.0f * dt.value());

    // Get the updated MVP matrix from the renderer
    mat4 mvp = rend->get_mvp_matrix(*this);

    // Update the uniform buffer
    mvp_buffer->update(mvp.data(), sizeof(float) * 16);
  }

  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::material> material;
  std::shared_ptr<mareweb::uniform_buffer> mvp_buffer;
  main_renderer *rend;
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

  app.create_renderer<main_renderer>(props);

  app.run();

  return 0;
}