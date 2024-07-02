#ifndef MAREWEB_APPLICATION_HPP
#define MAREWEB_APPLICATION_HPP

#include "mareweb/renderer.hpp"
#include <SDL3/SDL.h>
#include <memory>
#include <stdexcept>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class renderer;

class application {
public:
  static auto get_instance() -> application &;

  application(const application &) = delete;
  auto operator=(const application &) -> application & = delete;
  application(application &&) = delete;
  auto operator=(application &&) -> application & = delete;

  void initialize();
  void run();
  void quit();

  auto get_wgpu_instance() -> wgpu::Instance & { return m_instance; }
  auto get_wgpu_device() -> wgpu::Device & { return m_device; }

  template <typename T, typename... Args> void create_renderer(const renderer_properties &properties, Args &&...args);

private:
  application() = default;
  ~application();

  static void init_sdl();
  void init_webgpu();
  void handle_events();
  void handle_window_close(const SDL_Event& event);
  void handle_window_resize(const SDL_Event& event);
  void handle_key_event(const SDL_Event& event);
  void handle_mouse_button_event(const SDL_Event& event);
  void handle_mouse_motion_event(const SDL_Event& event);
  void handle_mouse_wheel_event(const SDL_Event& event);
  void setup_webgpu_callbacks(wgpu::DeviceDescriptor &device_desc);

  static auto create_window(const renderer_properties &properties) -> SDL_Window *;
  auto create_surface(SDL_Window *window) -> wgpu::Surface;

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