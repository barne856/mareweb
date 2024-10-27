#include "mareweb/texture.hpp"
#include <SDL_image.h>
#include <stdexcept>
#include <utility>

namespace mareweb {

texture::texture(wgpu::Device &device, const char *file_path) : m_device(device), m_owns_surface(true) {
  // Load image using SDL_image
  m_surface = IMG_Load(file_path);
  if (!m_surface) {
    throw std::runtime_error("Failed to load texture: " + std::string(IMG_GetError()));
  }

  m_width = m_surface->w;
  m_height = m_surface->h;
  m_format = wgpu::TextureFormat::RGBA8Unorm;

  create_texture_from_surface();
  create_sampler();
}

texture::texture(wgpu::Device &device, SDL_Surface *surface)
    : m_device(device), m_surface(surface), m_owns_surface(false) {
  if (!surface) {
    throw std::runtime_error("Invalid surface provided to texture constructor");
  }

  m_width = surface->w;
  m_height = surface->h;
  m_format = wgpu::TextureFormat::RGBA8Unorm;

  create_texture_from_surface();
  create_sampler();
}

texture::~texture() { cleanup(); }

texture::texture(texture &&other) noexcept
    : m_device(other.m_device), m_texture(std::exchange(other.m_texture, nullptr)),
      m_texture_view(std::exchange(other.m_texture_view, nullptr)), m_sampler(std::exchange(other.m_sampler, nullptr)),
      m_surface(std::exchange(other.m_surface, nullptr)), m_owns_surface(std::exchange(other.m_owns_surface, false)),
      m_width(std::exchange(other.m_width, 0)), m_height(std::exchange(other.m_height, 0)), m_format(other.m_format) {}

texture &texture::operator=(texture &&other) noexcept {
  if (this != &other) {
    cleanup();

    m_device = other.m_device;
    m_texture = std::exchange(other.m_texture, nullptr);
    m_texture_view = std::exchange(other.m_texture_view, nullptr);
    m_sampler = std::exchange(other.m_sampler, nullptr);
    m_surface = std::exchange(other.m_surface, nullptr);
    m_owns_surface = std::exchange(other.m_owns_surface, false);
    m_width = std::exchange(other.m_width, 0);
    m_height = std::exchange(other.m_height, 0);
    m_format = other.m_format;
  }
  return *this;
}

void texture::create_texture_from_surface() {
  // Create texture descriptor
  wgpu::TextureDescriptor texture_desc = {};
  texture_desc.size.width = m_width;
  texture_desc.size.height = m_height;
  texture_desc.size.depthOrArrayLayers = 1;
  texture_desc.mipLevelCount = 1;
  texture_desc.sampleCount = 1;
  texture_desc.dimension = wgpu::TextureDimension::e2D;
  texture_desc.format = m_format;
  texture_desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;

  m_texture = m_device.CreateTexture(&texture_desc);

#ifdef __EMSCRIPTEN__
  // Direct approach for Emscripten
  wgpu::ImageCopyTexture destination = {};
  destination.texture = m_texture;

  wgpu::TextureDataLayout data_layout = {};
  data_layout.offset = 0;
  data_layout.bytesPerRow = m_surface->pitch;
  data_layout.rowsPerImage = m_surface->h;

  wgpu::Extent3D write_size = {};
  write_size.width = m_surface->w;
  write_size.height = m_surface->h;
  write_size.depthOrArrayLayers = 1;

  m_device.GetQueue().WriteTexture(&destination, m_surface->pixels, m_surface->pitch * m_surface->h, &data_layout,
                                   &write_size);
#else
  // Native approach with conversion
  SDL_Surface *rgba_surface = SDL_ConvertSurfaceFormat(m_surface, SDL_PIXELFORMAT_RGBA32, 0);
  if (!rgba_surface) {
    throw std::runtime_error("Failed to convert texture to RGBA format");
  }

  // Write texture data
  wgpu::ImageCopyTexture destination = {};
  destination.texture = m_texture;

  wgpu::TextureDataLayout data_layout = {};
  data_layout.offset = 0;
  data_layout.bytesPerRow = rgba_surface->pitch;
  data_layout.rowsPerImage = rgba_surface->h;

  wgpu::Extent3D write_size = {};
  write_size.width = rgba_surface->w;
  write_size.height = rgba_surface->h;
  write_size.depthOrArrayLayers = 1;

  m_device.GetQueue().WriteTexture(&destination, rgba_surface->pixels, rgba_surface->pitch * rgba_surface->h,
                                   &data_layout, &write_size);

  SDL_FreeSurface(rgba_surface);
#endif

  // Create texture view
  wgpu::TextureViewDescriptor view_desc = {};
  view_desc.format = texture_desc.format;
  view_desc.dimension = wgpu::TextureViewDimension::e2D;
  view_desc.baseMipLevel = 0;
  view_desc.mipLevelCount = 1;
  view_desc.baseArrayLayer = 0;
  view_desc.arrayLayerCount = 1;
  view_desc.aspect = wgpu::TextureAspect::All;
  m_texture_view = m_texture.CreateView(&view_desc);
}

void texture::create_sampler(wgpu::AddressMode address_mode) {
  wgpu::SamplerDescriptor sampler_desc = {};
  sampler_desc.addressModeU = address_mode;
  sampler_desc.addressModeV = address_mode;
  sampler_desc.addressModeW = address_mode;
  sampler_desc.magFilter = wgpu::FilterMode::Linear;
  sampler_desc.minFilter = wgpu::FilterMode::Linear;
  sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
  sampler_desc.lodMinClamp = 0.0f;
  sampler_desc.lodMaxClamp = 1.0f;
  sampler_desc.compare = wgpu::CompareFunction::Undefined;
  sampler_desc.maxAnisotropy = 1;

  m_sampler = m_device.CreateSampler(&sampler_desc);
}

void texture::cleanup() {
  if (m_texture) {
    m_texture.Destroy();
    m_texture = nullptr;
  }

  if (m_surface && m_owns_surface) {
    SDL_FreeSurface(m_surface);
    m_surface = nullptr;
  }
}

} // namespace mareweb