#ifndef MAREWEB_RENDERER_HPP
#define MAREWEB_RENDERER_HPP

#include <SDL3/SDL.h>
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"

namespace mareweb {

struct renderer_properties {
  uint32_t width;
  uint32_t height;
  std::string title;
  bool fullscreen = false;
  bool resizable = true;
  wgpu::PresentMode present_mode = wgpu::PresentMode::Fifo; // Fifo is equivalent to VSync
  uint32_t sample_count = 1;                                // MSAA sample count
  wgpu::Color clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
};

class renderer {
public:
  renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, const renderer_properties &properties);
  virtual ~renderer();

  void resize(uint32_t new_width, uint32_t new_height);
  virtual void render() = 0;
  void present();

  std::unique_ptr<mesh> create_mesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices = {});
  std::unique_ptr<material> create_material(const std::string &vertex_shader_source,
                                            const std::string &fragment_shader_source);
  void set_fullscreen(bool fullscreen);
  void set_present_mode(wgpu::PresentMode present_mode);
  void set_clear_color(const wgpu::Color &clear_color) { m_clear_color = clear_color; }
  void begin_frame();
  void end_frame();
  void draw_mesh(const mesh &mesh, const material &material);

  SDL_Window *get_window() const { return m_window; }
  const std::string &get_title() const { return m_properties.title; }
  const renderer_properties &get_properties() const { return m_properties; }

protected:
  renderer_properties m_properties;
  SDL_Window *m_window;
  wgpu::Device m_device;
  wgpu::Surface m_surface;
  wgpu::TextureFormat m_surface_format;
  wgpu::CommandEncoder m_command_encoder;
  wgpu::RenderPassEncoder m_render_pass;
  wgpu::TextureView m_current_texture_view;
  wgpu::Color m_clear_color;
  wgpu::Texture m_msaa_texture;
  wgpu::TextureView m_msaa_texture_view;

private:
  void configure_surface();
  void create_msaa_texture();
};

} // namespace mareweb
#endif // MAREWEB_RENDERER_HPP