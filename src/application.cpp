#define SDL_MAIN_HANDLED
#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#elif defined(SDL_PLATFORM_LINUX)
#include <X11/Xlib.h>
#endif

namespace mareweb {
using namespace squint;

auto application::get_instance() -> application & {
  static application instance;
  return instance;
}

void application::initialize() {
  if (m_initialized) {
    return;
  }

  init_sdl();
  init_webgpu();

  m_initialized = true;
}

application::~application() {
  if (m_initialized) {
    SDL_Quit();
  }
}

void application::run() {
  if (!m_initialized) {
    throw std::runtime_error("Application not initialized");
  }

  static units::time dt_seconds{0.0F};
  static auto last_time = std::chrono::high_resolution_clock::now();

  while (!m_quit) {
    auto current_time = std::chrono::high_resolution_clock::now();
    dt_seconds = units::time(std::chrono::duration<float>(current_time - last_time).count());
    last_time = current_time;

    handle_events();

    // Update all renderers and their object hierarchies
    for (auto &rend : m_renderers) {
      // physics updates use a fixed time step
      rend->update(rend->get_properties().fixed_time_step);
    }

    // Render all renderers and their object hierarchies
    for (auto &rend : m_renderers) {
      // render updates use the actual time step
      rend->render(dt_seconds);
    }
  }
}

void application::quit() { m_quit = true; }

void application::init_sdl() {
  SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11,wayland,windows");
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
  }
}

void application::setup_webgpu_callbacks(wgpu::DeviceDescriptor &device_desc) {
  // Device lost callback
  wgpu::DeviceLostCallbackInfo device_lost_callback_info{};
  device_lost_callback_info.callback = [](WGPUDeviceImpl *const * /*device*/, WGPUDeviceLostReason reason,
                                          char const *message, void *userdata) {
    auto *self = static_cast<application *>(userdata);
    std::cerr << "Device lost: " << message << std::endl;

    std::string reason_str;
    switch (reason) {
    case WGPUDeviceLostReason_Destroyed:
      reason_str = "The device was explicitly destroyed";
      break;
    case WGPUDeviceLostReason_Unknown:
      reason_str = "The device was lost for an unknown reason";
      break;
    case WGPUDeviceLostReason_FailedCreation:
      reason_str = "The device was lost due to a failed creation";
      break;
    case WGPUDeviceLostReason_InstanceDropped:
      reason_str = "The instance was dropped";
      break;
    case WGPUDeviceLostReason_Force32:
      reason_str = "Force32";
      break;
    default:
      reason_str = "Unknown reason code: " + std::to_string(static_cast<int>(reason));
    }

    std::cerr << "Reason: " << reason_str << std::endl;
  };
  device_lost_callback_info.userdata = this;

  // Uncaptured error callback
  wgpu::UncapturedErrorCallbackInfo uncaptured_error_callback_info{};
  uncaptured_error_callback_info.callback = [](WGPUErrorType type, const char *message, void *userdata) {
    auto *self = static_cast<application *>(userdata);
    std::cerr << "Uncaptured error: " << message << std::endl;
    if (type == WGPUErrorType_DeviceLost) {
      self->m_quit = true;
    }
  };
  uncaptured_error_callback_info.userdata = this;

  // Set up the device descriptor with the callbacks
  device_desc.deviceLostCallbackInfo = device_lost_callback_info;
  device_desc.uncapturedErrorCallbackInfo = uncaptured_error_callback_info;
}

void application::init_webgpu() {
  m_instance = wgpu::CreateInstance();

  m_instance.RequestAdapter(
      nullptr,
      [](WGPURequestAdapterStatus status, WGPUAdapter c_adapter, const char * /*message*/, void *userdata) {
        auto *self = static_cast<application *>(userdata);
        if (status != WGPURequestAdapterStatus_Success) {
          throw std::runtime_error("Failed to request adapter");
        }
        wgpu::Adapter adapter = wgpu::Adapter::Acquire(c_adapter);

        // Create device descriptor with device lost callback info
        wgpu::DeviceDescriptor device_desc{};
        self->setup_webgpu_callbacks(device_desc);

        adapter.RequestDevice(
            &device_desc,
            [](WGPURequestDeviceStatus status, WGPUDevice c_device, const char * /*message*/, void *userdata) {
              auto *self = static_cast<application *>(userdata);
              if (status != WGPURequestDeviceStatus_Success) {
                throw std::runtime_error("Failed to request device");
              }
              self->m_device = wgpu::Device::Acquire(c_device);
            },
            self);
      },
      this);
}

void application::handle_events() {
  SDL_Event event{};
  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
    case SDL_QUIT:
      m_quit = true;
      break;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
        handle_window_close(event);
      } else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        handle_window_resize(event);
      }
      break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      handle_key_event(event);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      handle_mouse_button_event(event);
      break;
    case SDL_MOUSEMOTION:
      handle_mouse_motion_event(event);
      break;
    case SDL_MOUSEWHEEL:
      handle_mouse_wheel_event(event);
      break;
    }
  }
}

void application::handle_window_close(const SDL_Event &event) {
  auto it = std::find_if(m_renderers.begin(), m_renderers.end(), [&event](const auto &renderer) {
    return event.window.windowID == SDL_GetWindowID(renderer->get_window());
  });
  if (it != m_renderers.end()) {
    m_renderers.erase(it);
  }
  if (m_renderers.empty()) {
    m_quit = true;
  }
}

void application::handle_window_resize(const SDL_Event &event) {
  for (auto &rend : m_renderers) {
    if (event.window.windowID == SDL_GetWindowID(rend->get_window())) {
      rend->resize(event.window.data1, event.window.data2);
      break;
    }
  }
}

void application::handle_key_event(const SDL_Event &event) {
  auto key_evt = static_cast<key_event>(event.key.keysym.sym * 2 + (event.type == SDL_KEYDOWN ? 0 : 1));
  for (auto &rend : m_renderers) {
    if (rend->on_key(key_evt)) {
      break;
    }
  }
}

void application::handle_mouse_button_event(const SDL_Event &event) {
  auto button_evt =
      static_cast<mouse_button_event>(event.button.button * 2 + (event.type == SDL_MOUSEBUTTONDOWN ? 0 : 1));
  for (auto &rend : m_renderers) {
    if (rend->on_mouse_button(button_evt)) {
      break;
    }
  }
}

void application::handle_mouse_motion_event(const SDL_Event &event) {
  mouse_move_event move_evt{static_cast<float>(event.motion.x), static_cast<float>(event.motion.y),
                            static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)};
  for (auto &rend : m_renderers) {
    if (rend->on_mouse_move(move_evt)) {
      break;
    }
  }
}

void application::handle_mouse_wheel_event(const SDL_Event &event) {
  mouse_scroll_event scroll_evt{static_cast<float>(event.wheel.x), static_cast<float>(event.wheel.y)};
  for (auto &rend : m_renderers) {
    if (rend->on_mouse_wheel(scroll_evt)) {
      break;
    }
  }
}

auto application::create_window(const renderer_properties &properties) -> SDL_Window * {
  Uint32 flags = 0; // Start with no flags

  if (properties.resizable) {
    flags |= SDL_WINDOW_RESIZABLE;
  }

  SDL_Window *window = SDL_CreateWindow(properties.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        static_cast<int>(properties.width), static_cast<int>(properties.height), flags);

  if (window == nullptr) {
    throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
  }

  if (properties.fullscreen) {
    if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
      SDL_DestroyWindow(window);
      throw std::runtime_error(std::string("Failed to set fullscreen mode: ") + SDL_GetError());
    }
  }

  return window;
}

auto application::create_surface(SDL_Window *window) -> wgpu::Surface {
  wgpu::SurfaceDescriptor surface_descriptor{};

#if defined(_WIN32)
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(window, &wmi)) {
    throw std::runtime_error("Failed to get Win32 window info");
  }
  wgpu::SurfaceDescriptorFromWindowsHWND window_desc{};
  window_desc.hinstance = wmi.info.win.hinstance;
  window_desc.hwnd = wmi.info.win.window;
  surface_descriptor.nextInChain = &window_desc;
#elif defined(__linux__)
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(window, &wmi)) {
    throw std::runtime_error("Failed to get Linux window info");
  }
  if (wmi.subsystem == SDL_SYSWM_X11) {
    wgpu::SurfaceDescriptorFromXlibWindow window_desc{};
    window_desc.display = wmi.info.x11.display;
    window_desc.window = wmi.info.x11.window;
    surface_descriptor.nextInChain = &window_desc;
  } else if (wmi.subsystem == SDL_SYSWM_WAYLAND) {
    wgpu::SurfaceDescriptorFromWaylandSurface window_desc{};
    window_desc.display = wmi.info.wl.display;
    window_desc.surface = wmi.info.wl.surface;
    surface_descriptor.nextInChain = &window_desc;
  } else {
    throw std::runtime_error("Unsupported window system on Linux");
  }
#else
  throw std::runtime_error("Unsupported platform");
#endif

  return m_instance.CreateSurface(&surface_descriptor);
}

} // namespace mareweb