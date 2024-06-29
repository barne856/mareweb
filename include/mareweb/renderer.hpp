#ifndef MAREWEB_RENDERER_HPP
#define MAREWEB_RENDERER_HPP

#include <SDL3/SDL.h>
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"

namespace mareweb {

class Renderer {
public:
  Renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, uint32_t width, uint32_t height);
  virtual ~Renderer();

  void resize(uint32_t newWidth, uint32_t newHeight);
  virtual void render() = 0;
  void present();

  std::unique_ptr<Mesh> createMesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices = {});
  std::unique_ptr<Material> createMaterial(const std::string &vertexShaderSource,
                                           const std::string &fragmentShaderSource);
  void drawMesh(const Mesh &mesh, const Material &material);

  SDL_Window *getWindow() const { return m_window; }

protected:
  uint32_t m_width;
  uint32_t m_height;
  SDL_Window *m_window;
  wgpu::Device m_device;
  wgpu::Surface m_surface;

private:
  void configureSurface();
};

} // namespace mareweb
#endif // MAREWEB_RENDERER_HPP