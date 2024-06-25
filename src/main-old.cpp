#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <webgpu/webgpu_cpp.h>

#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

#if defined(SDL_PLATFORM_WIN32)
#include <windows.h>
#elif defined(SDL_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <wayland-client.h>
#endif

wgpu::Surface SDL_GetWGPUSurface(wgpu::Instance instance, SDL_Window *window) {
  SDL_PropertiesID propertiesID = SDL_GetWindowProperties(window);
  if (propertiesID == 0) {
    throw std::runtime_error("SDL_GetWindowProperties failed");
  }

  wgpu::SurfaceDescriptor surfaceDescriptor = {};

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
    Display *display = static_cast<Display *>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL));
    Window x11Window = static_cast<Window>(SDL_GetNumberProperty(propertiesID, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
    if (display && x11Window != 0) {
      wgpu::SurfaceDescriptorFromXlibWindow windowDesc{};
      windowDesc.display = display;
      windowDesc.window = x11Window;
      surfaceDescriptor.nextInChain = &windowDesc;
    } else {
      throw std::runtime_error("Failed to get X11 window properties");
    }
  } else if (std::strcmp(videoDriver, "wayland") == 0) {
    struct wl_display *display =
        static_cast<struct wl_display *>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL));
    struct wl_surface *surface =
        static_cast<struct wl_surface *>(SDL_GetProperty(propertiesID, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL));
    if (display && surface) {
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

  surfaceDescriptor.label = NULL;
  return instance.CreateSurface(&surfaceDescriptor);
}

SDL_Window *window = nullptr;
bool quit = false;
wgpu::Instance instance;
wgpu::Device device;
wgpu::Surface surface;
wgpu::SwapChain swapChain;
wgpu::RenderPipeline pipeline;
const uint32_t initWidth = 512;
const uint32_t initHeight = 512;

void GetDevice(void (*callback)(wgpu::Device)) {
  instance.RequestAdapter(
      nullptr,
      // TODO(https://bugs.chromium.org/p/dawn/issues/detail?id=1892): Use
      // wgpu::RequestAdapterStatus, wgpu::Adapter, and wgpu::Device.
      [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *userdata) {
        if (status != WGPURequestAdapterStatus_Success) {
          exit(0);
        }
        wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
        adapter.RequestDevice(
            nullptr,
            [](WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata) {
              wgpu::Device device = wgpu::Device::Acquire(cDevice);
              reinterpret_cast<void (*)(wgpu::Device)>(userdata)(device);
            },
            userdata);
      },
      reinterpret_cast<void *>(callback));
}

const char shaderCode[] = R"(
    @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
      @builtin(position) vec4f {
        const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
        return vec4f(pos[i], 0, 1);
    }
    @fragment fn fragmentMain() -> @location(0) vec4f {
        return vec4f(1, 0, 0, 1);
    }
)";

void CreateRenderPipeline() {
  wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
  wgslDesc.code = shaderCode;

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain = &wgslDesc};
  wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

  wgpu::ColorTargetState colorTargetState{.format = wgpu::TextureFormat::BGRA8Unorm};

  wgpu::FragmentState fragmentState{
      .module = shaderModule, .entryPoint = "fragmentMain", .targetCount = 1, .targets = &colorTargetState};

  wgpu::RenderPipelineDescriptor descriptor{.vertex = {.module = shaderModule, .entryPoint = "vertexMain"},
                                            .fragment = &fragmentState};
  pipeline = device.CreateRenderPipeline(&descriptor);
}

void mainLoop() {
  // Add your main loop code here
  // For example, check for SDL events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      quit = true;
      break;
    }
  }

  static float time = 0.0f;
  time += 0.01f;

  wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
  wgpu::TextureView backbuffer = swapChain.GetCurrentTextureView();
  wgpu::Color clearColor = {std::sin(time + 1.0), std::cos(time), std::sin(time), 1.0f};
  wgpu::RenderPassColorAttachment colorAttachment = {
      .view = backbuffer,
      .resolveTarget = nullptr,
      .loadOp = wgpu::LoadOp::Clear,
      .storeOp = wgpu::StoreOp::Store,
      .clearValue = clearColor,
  };
  wgpu::RenderPassDescriptor renderPassDescriptor = {
      .nextInChain = nullptr,
      .label = "Render Pass",
      .colorAttachmentCount = 1,
      .colorAttachments = &colorAttachment,
      .depthStencilAttachment = nullptr,
  };
  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
  pass.SetPipeline(pipeline);
  pass.Draw(3);
  pass.End();
  wgpu::CommandBuffer commands = encoder.Finish();
  device.GetQueue().Submit(1, &commands);
}

void Start() {
  SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland,windows");
  SDL_SetMainReady();
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    throw std::runtime_error("SDL initialization failed");
  }

  // Create a resizable window
  window = SDL_CreateWindow("Resizable Window", initWidth, initHeight, SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    throw std::runtime_error("Window creation failed");
  }

  surface = SDL_GetWGPUSurface(instance, window);

  wgpu::SwapChainDescriptor scDesc{.usage = wgpu::TextureUsage::RenderAttachment,
                                   .format = wgpu::TextureFormat::BGRA8Unorm,
                                   .width = uint32_t(initWidth),
                                   .height = uint32_t(initHeight),
                                   .presentMode = wgpu::PresentMode::Fifo};
  swapChain = device.CreateSwapChain(surface, &scDesc);

  CreateRenderPipeline();

  // Main loop
  while (!quit) {
    mainLoop(); // Call your main loop logic
    swapChain.Present();
  }

  // Cleanup
  // must be destroyed after swap chain/surface
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main() {
  instance = wgpu::CreateInstance();
  GetDevice([](wgpu::Device dev) {
    device = dev;
    Start();
  });
}
