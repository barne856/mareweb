#ifndef MAREWEB_APPLICATION_HPP
#define MAREWEB_APPLICATION_HPP

#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include "mareweb/renderer.hpp"

namespace mareweb {

class Renderer;

class Application {
public:
  static Application &getInstance();

  void initialize();
  void run();
  void quit();

  wgpu::Instance &getWGPUInstance() { return m_instance; }
  wgpu::Device &getWGPUDevice() { return m_device; }

  template <typename T, typename... Args> void createRenderer(const RendererProperties &properties, Args &&...args);

private:
  Application() = default;
  ~Application();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;

  void initSDL();
  void initWebGPU();
  void handleEvents();
  void setupWebGPUCallbacks(wgpu::DeviceDescriptor& deviceDesc);

  SDL_Window *createWindow(const RendererProperties &properties);
  wgpu::Surface createSurface(SDL_Window *window);

  bool m_initialized = false;
  bool m_quit = false;
  wgpu::Instance m_instance;
  wgpu::Device m_device;
  std::vector<std::unique_ptr<Renderer>> m_renderers;
};

template <typename T, typename... Args>
void Application::createRenderer(const RendererProperties &properties, Args &&...args) {
  if (!m_initialized) {
    throw std::runtime_error("Application not initialized");
  }

  SDL_Window *window = createWindow(properties);
  wgpu::Surface surface = createSurface(window);

  m_renderers.push_back(std::make_unique<T>(m_device, surface, window, properties, std::forward<Args>(args)...));
}

} // namespace mareweb
#endif // MAREWEB_APPLICATION_HPP