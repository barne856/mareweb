#define SDL_MAIN_HANDLED
#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <stdexcept>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#elif defined(SDL_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <wayland-client.h>
#endif

namespace mareweb {

Application &Application::getInstance() {
  static Application instance;
  return instance;
}

void Application::initialize() {
  if (m_initialized)
    return;

  initSDL();
  initWebGPU();

  m_initialized = true;
}

Application::~Application() {
  if (m_initialized) {
    SDL_Quit();
  }
}

void Application::run() {
  if (!m_initialized) {
    throw std::runtime_error("Application not initialized");
  }

  while (!m_quit) {
    handleEvents();
    for (auto &renderer : m_renderers) {
      renderer->beginFrame();
      renderer->render();
      renderer->endFrame();
    }
  }
}

void Application::quit() { m_quit = true; }

void Application::initSDL() {
  SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland,windows");
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
  }
}

void Application::initWebGPU() {
  m_instance = wgpu::CreateInstance();

  m_instance.RequestAdapter(
      nullptr,
      [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char * /*message*/, void *userdata) {
        auto *self = static_cast<Application *>(userdata);
        if (status != WGPURequestAdapterStatus_Success) {
          throw std::runtime_error("Failed to request adapter");
        }
        wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);

        // Create device descriptor with device lost callback info
        wgpu::DeviceDescriptor deviceDesc{};
        wgpu::DeviceLostCallbackInfo deviceLostCallbackInfo{};
        deviceLostCallbackInfo.callback = [](WGPUDeviceImpl *const * /*device*/, WGPUDeviceLostReason /*reason*/,
                                             char const *message, void *userdata) {
          auto *self = static_cast<Application *>(userdata);
          std::cerr << "Device lost: " << message << std::endl;
          self->m_quit = true;
        };
        deviceLostCallbackInfo.userdata = self;
        deviceDesc.deviceLostCallbackInfo = deviceLostCallbackInfo;

        // Add uncaptured error callback info
        wgpu::UncapturedErrorCallbackInfo uncapturedErrorCallbackInfo{};
        uncapturedErrorCallbackInfo.callback = [](WGPUErrorType /*type*/, const char *message, void *userdata) {
          auto *self = static_cast<Application *>(userdata);
          std::cerr << "Uncaptured error: " << message << std::endl;
          self->m_quit = true;
        };
        uncapturedErrorCallbackInfo.userdata = self;
        deviceDesc.uncapturedErrorCallbackInfo = uncapturedErrorCallbackInfo;

        adapter.RequestDevice(
            &deviceDesc,
            [](WGPURequestDeviceStatus status, WGPUDevice cDevice, const char * /*message*/, void *userdata) {
              auto *self = static_cast<Application *>(userdata);
              if (status != WGPURequestDeviceStatus_Success) {
                throw std::runtime_error("Failed to request device");
              }
              self->m_device = wgpu::Device::Acquire(cDevice);
            },
            self);
      },
      this);
}

void Application::handleEvents() {
  SDL_Event event{};
  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      m_quit = true;
      break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
      auto it = std::find_if(m_renderers.begin(), m_renderers.end(), [&event](const auto &renderer) {
        return event.window.windowID == SDL_GetWindowID(renderer->getWindow());
      });
      if (it != m_renderers.end()) {
        m_renderers.erase(it);
      }
      if (m_renderers.empty()) {
        m_quit = true;
      }
    } break;
    case SDL_EVENT_WINDOW_RESIZED:
      for (auto &renderer : m_renderers) {
        if (event.window.windowID == SDL_GetWindowID(renderer->getWindow())) {
          renderer->resize(event.window.data1, event.window.data2);
          break;
        }
      }
      break;
    }
  }
}

SDL_Window *Application::createWindow(uint32_t width, uint32_t height) {
  SDL_Window *window =
      SDL_CreateWindow("WebGPU Window", static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
  }
  return window;
}

wgpu::Surface Application::createSurface(SDL_Window *window) {
  SDL_PropertiesID propertiesID = SDL_GetWindowProperties(window);
  if (propertiesID == 0) {
    throw std::runtime_error("SDL_GetWindowProperties failed");
  }

  wgpu::SurfaceDescriptor surfaceDescriptor{};

#if defined(SDL_PLATFORM_WIN32)
  HWND hwnd = static_cast<HWND>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL));
  HINSTANCE hinstance =
      static_cast<HINSTANCE>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL));
  if (hwnd && hinstance) {
    wgpu::SurfaceDescriptorFromWindowsHWND windowDesc{};
    windowDesc.hinstance = hinstance;
    windowDesc.hwnd = hwnd;
    surfaceDescriptor.nextInChain = &windowDesc;
  } else {
    throw std::runtime_error("Failed to get Win32 window properties");
  }
#elif defined(SDL_PLATFORM_LINUX)
  const char *videoDriver = SDL_GetCurrentVideoDriver();
  if (std::strcmp(videoDriver, "x11") == 0) {
    auto *display = static_cast<Display *>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
    auto x11Window = static_cast<Window>(SDL_GetNumberProperty(propertiesID, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
    if ((display != nullptr) && x11Window != 0) {
      wgpu::SurfaceDescriptorFromXlibWindow windowDesc{};
      windowDesc.display = display;
      windowDesc.window = x11Window;
      surfaceDescriptor.nextInChain = &windowDesc;
    } else {
      throw std::runtime_error("Failed to get X11 window properties");
    }
  } else if (std::strcmp(videoDriver, "wayland") == 0) {
    auto *display = static_cast<struct wl_display *>(
        SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
    auto *surface = static_cast<struct wl_surface *>(
        SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));
    if ((display != nullptr) && (surface != nullptr)) {
      wgpu::SurfaceDescriptorFromWaylandSurface windowDesc{};
      windowDesc.display = display;
      windowDesc.surface = surface;
      surfaceDescriptor.nextInChain = &windowDesc;
    } else {
      throw std::runtime_error("Failed to get Wayland window properties");
    }
  } else {
    throw std::runtime_error("Unsupported video driver on Linux");
  }
#else
  throw std::runtime_error("Unsupported platform");
#endif

  return m_instance.CreateSurface(&surfaceDescriptor);
}

} // namespace mareweb