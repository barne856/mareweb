#include "webgpu/webgpu_cpp.h"
#define SDL_MAIN_HANDLED
#include "mareweb/application.hpp"
#include "mareweb/renderer.hpp"
#include "squint/quantity.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <SDL_image.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#elif defined(SDL_PLATFORM_LINUX)
#include <X11/Xlib.h>
#endif
#undef Success // Xlib.h defines this dumb macro, which conflicts with the enum value in dawn

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
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
  std::cout << "Goodbye!" << std::endl;
  if (m_initialized) {
    m_renderers.clear();
    m_device.Destroy();
    SDL_Quit();
  }
}

void application::on_frame() {
  static squint::duration dt_seconds{0.0F};
  static auto last_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  dt_seconds = squint::duration(std::chrono::duration<float>(current_time - last_time).count());
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

void application::run() {
  if (!m_initialized) {
    throw std::runtime_error("Application not initialized");
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(
      [](void *userData) {
        application &app = *reinterpret_cast<application *>(userData);
        app.on_frame();
      },
      (void *)this, 0, true);
#else
  while (!m_quit) {
    on_frame();
  }
#endif
}

void application::quit() { m_quit = true; }

void application::init_sdl() {
#ifndef __EMSCRIPTEN__
  SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11,wayland,windows");
#endif
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
  }
  // init sdl image for png and jpg support
  if ((IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) & (IMG_INIT_JPG | IMG_INIT_PNG)) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
    throw std::runtime_error(std::string("SDL image initialization failed: ") + IMG_GetError());
  }
}

void application::setup_webgpu_callbacks(wgpu::DeviceDescriptor &device_desc) {
#ifdef __EMSCRIPTEN__
  // // Device lost callback
  // wgpu::DeviceLostCallbackInfo device_lost_callback_info{};
  // device_lost_callback_info.callback = [](WGPUDeviceImpl *const * /*device*/, WGPUDeviceLostReason reason,
  //                                         char const *message, void *userdata) {
  //   auto *self = static_cast<application *>(userdata);
  //   std::cerr << "Device lost: " << message << std::endl;
  //
  //   std::string reason_str;
  //   switch (reason) {
  //   case WGPUDeviceLostReason_Destroyed:
  //     reason_str = "The device was explicitly destroyed";
  //     break;
  //   case WGPUDeviceLostReason_Unknown:
  //     reason_str = "The device was lost for an unknown reason";
  //     break;
  //   case WGPUDeviceLostReason_FailedCreation:
  //     reason_str = "The device was lost due to a failed creation";
  //     break;
  //   case WGPUDeviceLostReason_InstanceDropped:
  //     reason_str = "The instance was dropped";
  //     break;
  //   case WGPUDeviceLostReason_Force32:
  //     reason_str = "Force32";
  //     break;
  //   default:
  //     reason_str = "Unknown reason code: " + std::to_string(static_cast<int>(reason));
  //   }
  //
  //   std::cerr << "Reason: " << reason_str << std::endl;
  // };
  // device_lost_callback_info.userdata = this;

  // // Uncaptured error callback
  // wgpu::UncapturedErrorCallbackInfo uncaptured_error_callback_info{};
  // uncaptured_error_callback_info.callback = [](WGPUErrorType type, const char *message, void *userdata) {
  //   auto *self = static_cast<application *>(userdata);
  //   std::cerr << "Uncaptured error: " << message << std::endl;
  //   if (type == WGPUErrorType_DeviceLost) {
  //     self->m_quit = true;
  //   }
  // };
  // uncaptured_error_callback_info.userdata = this;

  // Set up the device descriptor with the callbacks
  // device_desc.deviceLostCallbackInfo = device_lost_callback_info;
  // device_desc.uncapturedErrorCallbackInfo = uncaptured_error_callback_info;
#else
  // Device lost callback
  device_desc.SetDeviceLostCallback(
      wgpu::CallbackMode::WaitAnyOnly,
      [](const wgpu::Device & /*device*/, wgpu::DeviceLostReason reason, wgpu::StringView message,
         application * /*app*/) {
        std::cerr << "Device lost: " << message.data << std::endl;

        std::string reason_str;
        switch (reason) {
        case wgpu::DeviceLostReason::Destroyed:
          reason_str = "The device was explicitly destroyed";
          break;
        case wgpu::DeviceLostReason::Unknown:
          reason_str = "The device was lost for an unknown reason";
          break;
        case wgpu::DeviceLostReason::FailedCreation:
          reason_str = "The device was lost due to a failed creation";
          break;
        case wgpu::DeviceLostReason::InstanceDropped:
          reason_str = "The instance was dropped";
          break;
        default:
          reason_str = "Unknown reason code: " + std::to_string(static_cast<int>(reason));
        }
        std::cerr << "Reason: " << reason_str << std::endl;
      },
      this);

  device_desc.SetUncapturedErrorCallback(
      [](const wgpu::Device & /*device*/, wgpu::ErrorType type, wgpu::StringView message, application *app) {
        std::cerr << "Uncaptured error: " << message.data << std::endl;
        throw std::runtime_error("Uncaptured error: " + std::string(message.data));
        if (type == wgpu::ErrorType::DeviceLost) {
          app->m_quit = true;
        }
      },
      this);
#endif
}

void application::init_webgpu() {
#ifdef __EMSCRIPTEN__
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
  // Wait for the device to be created
  while (m_device.Get() == nullptr) {
    emscripten_sleep(100);
  }
#else
  m_instance = wgpu::CreateInstance();

  wgpu::Future adapter_future = m_instance.RequestAdapter(
      nullptr, wgpu::CallbackMode::WaitAnyOnly,
      [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message, application *app) {
        if (status != wgpu::RequestAdapterStatus::Success) {
          throw std::runtime_error("Failed to request adapter: " + std::string(message.data));
        }

        // Create device descriptor with device lost callback info
        wgpu::DeviceDescriptor device_desc{};
        app->setup_webgpu_callbacks(device_desc);

        wgpu::Future device_future = adapter.RequestDevice(
            &device_desc, wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message, application *app) {
              if (status != wgpu::RequestDeviceStatus::Success) {
                throw std::runtime_error("Failed to request device: " + std::string(message.data));
              }
              app->m_device = device;
            },
            app);
        auto device_wait_status = app->m_instance.WaitAny(device_future, 0);
        if (device_wait_status != wgpu::WaitStatus::Success) {
          if (device_wait_status == wgpu::WaitStatus::TimedOut) {
            throw std::runtime_error("Device creation timed out");
          }
          if (device_wait_status == wgpu::WaitStatus::UnsupportedTimeout) {
            throw std::runtime_error("Device creation timeout not supported");
          }
          if (device_wait_status == wgpu::WaitStatus::UnsupportedCount) {
            throw std::runtime_error("Device creation count not supported");
          }
          if (device_wait_status == wgpu::WaitStatus::UnsupportedMixedSources) {
            throw std::runtime_error("Device creation mixed sources not supported");
          }
          if (device_wait_status == wgpu::WaitStatus::Unknown) {
            throw std::runtime_error("Device creation unknown error");
          }
          throw std::runtime_error("Failed to wait for device creation");
        }
      },
      this);
  auto adapter_wait_status = m_instance.WaitAny(adapter_future, 0);
  if (adapter_wait_status != wgpu::WaitStatus::Success) {
    if (adapter_wait_status == wgpu::WaitStatus::TimedOut) {
      throw std::runtime_error("Adapter creation timed out");
    }
    if (adapter_wait_status == wgpu::WaitStatus::UnsupportedTimeout) {
      throw std::runtime_error("Adapter creation timeout not supported");
    }
    if (adapter_wait_status == wgpu::WaitStatus::UnsupportedCount) {
      throw std::runtime_error("Adapter creation count not supported");
    }
    if (adapter_wait_status == wgpu::WaitStatus::UnsupportedMixedSources) {
      throw std::runtime_error("Adapter creation mixed sources not supported");
    }
    if (adapter_wait_status == wgpu::WaitStatus::Unknown) {
      throw std::runtime_error("Adapter creation unknown error");
    }
    throw std::runtime_error("Failed to wait for adapter creation");
  }
#endif
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
      window_resize_event resize_evt{static_cast<uint64_t>(event.window.data1),
                                     static_cast<uint64_t>(event.window.data2)};
      if (rend->on_resize(resize_evt)) {
        break;
      }
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
  if (SDL_GetWindowWMInfo(window, &wmi) == 0U) {
    throw std::runtime_error("Failed to get Linux window info");
  }
#ifdef SDL_VIDEO_DRIVER_X11
  wgpu::SurfaceDescriptorFromXlibWindow window_desc{};
  window_desc.display = wmi.info.x11.display;
  window_desc.window = wmi.info.x11.window;
  surface_descriptor.nextInChain = &window_desc;
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
  wgpu::SurfaceDescriptorFromWaylandSurface window_desc{};
  window_desc.display = wmi.info.wl_display;
  window_desc.surface = wmi.info.wl_surface;
  surface_descriptor.nextInChain = &window_desc;
#else
  throw std::runtime_error("Unsupported window system on Linux");
#endif
#elif defined(SDL_VIDEO_DRIVER_EMSCRIPTEN)
  wgpu::SurfaceDescriptorFromCanvasHTMLSelector window_desc{};
  window_desc.selector = "#canvas";
  surface_descriptor.nextInChain = &window_desc;
#else
  throw std::runtime_error("Unsupported platform");
#endif

  return m_instance.CreateSurface(&surface_descriptor);
}

} // namespace mareweb