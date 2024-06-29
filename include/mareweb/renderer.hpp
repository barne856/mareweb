#ifndef MAREWEB_RENDERER_HPP
#define MAREWEB_RENDERER_HPP

#include <SDL3/SDL.h>
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class Renderer {
public:
  Renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, uint32_t width, uint32_t height);
  virtual ~Renderer();

  void resize(uint32_t newWidth, uint32_t newHeight);
  virtual void render() = 0;
  void present();

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