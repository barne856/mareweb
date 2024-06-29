#include "mareweb/renderer.hpp"
#include <stdexcept>

namespace mareweb {

Renderer::Renderer(wgpu::Device& device, wgpu::Surface surface, SDL_Window* window, uint32_t width, uint32_t height)
    : m_device(device), m_surface(surface), m_window(window), m_width(width), m_height(height) {
    configureSurface();
}

Renderer::~Renderer() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
}

void Renderer::resize(uint32_t newWidth, uint32_t newHeight) {
    m_width = newWidth;
    m_height = newHeight;
    configureSurface();
}

void Renderer::present() {
    m_surface.Present();
}

void Renderer::configureSurface() {
    wgpu::SurfaceCapabilities capabilities{};
    m_surface.GetCapabilities(m_device.GetAdapter(), &capabilities);
    wgpu::TextureFormat format = *capabilities.formats;

    wgpu::SurfaceConfiguration config{};
    config.device = m_device;
    config.format = format;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.width = m_width;
    config.height = m_height;
    config.presentMode = wgpu::PresentMode::Fifo;

    m_surface.Configure(&config);
}

} // namespace mareweb