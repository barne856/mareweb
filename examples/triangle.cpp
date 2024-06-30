#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <iostream>
#include <vector>

class triangle_renderer : public mareweb::renderer {
public:
  triangle_renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                    const mareweb::renderer_properties &properties)
      : renderer(device, surface, window, properties) {
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

    m_mesh = create_mesh(vertices);
    m_material = create_material(vertex_shader_source, fragment_shader_source);
    set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
  }

  ~triangle_renderer() override = default; // The default destructor will clean up m_mesh and m_material

  void render(float dt) override { draw_mesh(*m_mesh, *m_material); }

private:
  std::unique_ptr<mareweb::mesh> m_mesh;
  std::unique_ptr<mareweb::material> m_material;
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

  app.create_renderer<triangle_renderer>(props);

  app.run();

  return 0;
}