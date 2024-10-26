// texture.hpp
#ifndef MAREWEB_TEXTURE_HPP
#define MAREWEB_TEXTURE_HPP

#include <SDL2/SDL_surface.h>
#include <string>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class texture {
public:
  texture(wgpu::Device &device, const char *file_path);
  texture(wgpu::Device &device, SDL_Surface *surface);
  ~texture();

  // Delete copy operations
  texture(const texture &) = delete;
  texture &operator=(const texture &) = delete;

  // Allow move operations
  texture(texture &&other) noexcept;
  texture &operator=(texture &&other) noexcept;

  [[nodiscard]] auto get_texture_view() const -> wgpu::TextureView { return m_texture_view; }
  [[nodiscard]] auto get_sampler() const -> wgpu::Sampler { return m_sampler; }
  [[nodiscard]] auto get_width() const -> int { return m_width; }
  [[nodiscard]] auto get_height() const -> int { return m_height; }
  [[nodiscard]] auto get_format() const -> wgpu::TextureFormat { return m_format; }

private:
  wgpu::Device m_device;
  wgpu::Texture m_texture;
  wgpu::TextureView m_texture_view;
  wgpu::Sampler m_sampler;
  SDL_Surface *m_surface = nullptr;
  bool m_owns_surface = false;
  int m_width = 0;
  int m_height = 0;
  wgpu::TextureFormat m_format;

  void create_texture_from_surface();
  void create_sampler(wgpu::AddressMode address_mode = wgpu::AddressMode::Repeat);
  void cleanup();
};

} // namespace mareweb

#endif // MAREWEB_TEXTURE_HPP