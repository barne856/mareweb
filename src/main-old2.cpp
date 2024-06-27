#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#elif defined(SDL_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <wayland-client.h>
#endif

class Renderer {
public:
  Renderer(wgpu::Device &device, wgpu::Surface &surface, uint32_t width, uint32_t height)
      : m_device(device), m_surface(surface), m_width(width), m_height(height), m_pipeline() {
    configureSurface();
    createRenderPipeline();
  }

  void resize(uint32_t newWidth, uint32_t newHeight) {
    m_width = newWidth;
    m_height = newHeight;
    configureSurface();
  }

  void render(float time) const {
    wgpu::SurfaceTexture surfaceTexture{};
    m_surface.GetCurrentTexture(&surfaceTexture);
    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder();

    const wgpu::Color clearColor{std::sin(time + 1.0F), std::cos(time), std::sin(time), 1.0F};
    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = view;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = clearColor;

    wgpu::RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
    pass.SetPipeline(m_pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    m_device.GetQueue().Submit(1, &commands);
  }

  void present() const { m_surface.Present(); }

private:
  static void configureSurface(const wgpu::Device &device, wgpu::Surface &surface, uint32_t width, uint32_t height) {
    wgpu::SurfaceCapabilities capabilities{};
    surface.GetCapabilities(device.GetAdapter(), &capabilities);
    wgpu::TextureFormat format = *capabilities.formats;

    wgpu::SurfaceConfiguration config{};
    config.device = device;
    config.format = format;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.width = width;
    config.height = height;
    config.presentMode = wgpu::PresentMode::Fifo;

    surface.Configure(&config);
  }

  void configureSurface() { configureSurface(m_device, m_surface, m_width, m_height); }

  static void createRenderPipeline(const wgpu::Device &device, wgpu::Surface &surface, wgpu::RenderPipeline &pipeline) {
    constexpr std::string_view shaderCode = R"(
        @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
          @builtin(position) vec4f {
            const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
            return vec4f(pos[i], 0, 1);
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(1, 0, 0, 1);
        }
    )";

    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = shaderCode.data();

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{};
    shaderModuleDescriptor.nextInChain = &wgslDesc;

    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    wgpu::ColorTargetState colorTargetState{};
    wgpu::SurfaceCapabilities capabilities{};
    surface.GetCapabilities(device.GetAdapter(), &capabilities);
    colorTargetState.format = *capabilities.formats;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fragmentMain";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::RenderPipelineDescriptor descriptor{};
    descriptor.vertex.module = shaderModule;
    descriptor.vertex.entryPoint = "vertexMain";
    descriptor.fragment = &fragmentState;

    pipeline = device.CreateRenderPipeline(&descriptor);
  }

  void createRenderPipeline() { createRenderPipeline(m_device, m_surface, m_pipeline); }

  wgpu::Device &m_device;
  wgpu::Surface &m_surface;
  uint32_t m_width;
  uint32_t m_height;
  wgpu::RenderPipeline m_pipeline;
};

class Application {
public:
  Application(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
    initSDL();
    initWindow();
    initWebGPU();
  }

  ~Application() {
    SDL_DestroyWindow(m_window);
    SDL_Quit();
  }

  Application(const Application &) = delete;
  auto operator=(const Application &) -> Application & = delete;
  Application(Application &&) = delete;
  auto operator=(Application &&) -> Application & = delete;

  void run() {
    m_renderer = std::make_unique<Renderer>(m_device, m_surface, m_width, m_height);

    float time = 0.0F;
    constexpr float TIME_STEP = 0.01F;
    while (!m_quit) {
      handleEvents();
      time += TIME_STEP;
      m_renderer->render(time);
      m_renderer->present();
    }
  }

private:
  static void initSDL() {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland,windows");
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
    }
  }

  static void initWindow(uint32_t width, uint32_t height, SDL_Window *&window) {
    window = SDL_CreateWindow("WebGPU Window", static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
      SDL_Quit();
      throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
    }
  }

  void initWindow() { initWindow(m_width, m_height, m_window); }

  void initWebGPU() {
    m_instance = wgpu::CreateInstance();
    m_surface = getWGPUSurface(m_instance, m_window);

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

  void handleEvents() {
    SDL_Event event{};
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        m_quit = true;
        break;
      case SDL_EVENT_WINDOW_RESIZED:
        m_width = static_cast<uint32_t>(event.window.data1);
        m_height = static_cast<uint32_t>(event.window.data2);
        if (m_renderer) {
          m_renderer->resize(m_width, m_height);
        }
        break;
      }
    }
  }

  static auto getWGPUSurface(wgpu::Instance &instance, SDL_Window *window) -> wgpu::Surface {
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
      auto *display =
          static_cast<Display *>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
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

    return instance.CreateSurface(&surfaceDescriptor);
  }

  uint32_t m_width;
  uint32_t m_height;
  bool m_quit{false};
  SDL_Window *m_window{nullptr};
  wgpu::Instance m_instance{};
  wgpu::Device m_device{};
  wgpu::Surface m_surface{};
  std::unique_ptr<Renderer> m_renderer{nullptr};
};

auto main() -> int {
  constexpr uint32_t WINDOW_WIDTH = 512;
  constexpr uint32_t WINDOW_HEIGHT = 512;
  try {
    Application app(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}