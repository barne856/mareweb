#ifndef MAREWEB_APPLICATION_HPP
#define MAREWEB_APPLICATION_HPP

#include "mareweb/renderer.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <stdexcept>

namespace mareweb {

class renderer;

class application {
public:
  static application &get_instance();

  void initialize();
  void run();
  void quit();

  wgpu::Instance &get_wgpu_instance() { return m_instance; }
  wgpu::Device &get_wgpu_device() { return m_device; }

  template <typename T, typename... Args> void create_renderer(const renderer_properties &properties, Args &&...args);

private:
  application() = default;
  ~application();

  application(const application &) = delete;
  application &operator=(const application &) = delete;
  application(application &&) = delete;
  application &operator=(application &&) = delete;

  void init_sdl();
  void init_webgpu();
  void handle_events();
  void setup_webgpu_callbacks(wgpu::DeviceDescriptor &device_desc);

  SDL_Window *create_window(const renderer_properties &properties);
  wgpu::Surface create_surface(SDL_Window *window);

  void update_object_hierarchy(object *obj, float dt);
  void render_object_hierarchy(renderer *rend, float dt);
  void render_object(object *obj, float dt);

  bool m_initialized = false;
  bool m_quit = false;
  wgpu::Instance m_instance;
  wgpu::Device m_device;
  std::vector<std::unique_ptr<renderer>> m_renderers;
};

template <typename T, typename... Args>
void application::create_renderer(const renderer_properties &properties, Args &&...args) {
  if (!m_initialized) {
    throw std::runtime_error("Application not initialized");
  }

  SDL_Window *window = create_window(properties);
  wgpu::Surface surface = create_surface(window);

  m_renderers.push_back(std::make_unique<T>(m_device, surface, window, properties, std::forward<Args>(args)...));
}

} // namespace mareweb
#endif // MAREWEB_APPLICATION_HPP