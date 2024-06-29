#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <vector>

class TriangleRenderer : public mareweb::Renderer {
public:
  TriangleRenderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                   const mareweb::RendererProperties &properties)
      : Renderer(device, surface, window, properties) {
    std::vector<float> vertices = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};

    const char *vertexShaderSource = R"(
            @vertex
            fn main(@location(0) position: vec3<f32>) -> @builtin(position) vec4<f32> {
                return vec4<f32>(position, 1.0);
            }
        )";

    const char *fragmentShaderSource = R"(
            @fragment
            fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(1.0, 0.1, 0.05, 1.0);
            }
        )";

    m_mesh = createMesh(vertices);
    m_material = createMaterial(vertexShaderSource, fragmentShaderSource);
    setClearColor({0.05f, 0.05f, 0.05f, 1.0f});
    setFullscreen(true);
  }

  ~TriangleRenderer() override = default; // The default destructor will clean up m_mesh and m_material

  void render() override { drawMesh(*m_mesh, *m_material); }

private:
  std::unique_ptr<mareweb::Mesh> m_mesh;
  std::unique_ptr<mareweb::Material> m_material;
};

int main() {
  mareweb::Application &app = mareweb::Application::getInstance();
  app.initialize();

  mareweb::RendererProperties props = {.width = 800,
                                       .height = 600,
                                       .title = "Triangle",
                                       .fullscreen = true,
                                       .resizable = true,
                                       .presentMode = wgpu::PresentMode::Fifo,
                                       .sampleCount = 4};

  app.createRenderer<TriangleRenderer>(props);

  app.run();

  return 0;
}