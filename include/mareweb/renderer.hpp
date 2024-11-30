#ifndef MAREWEB_RENDERER_HPP
#define MAREWEB_RENDERER_HPP

#include <SDL2/SDL.h>
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

#include "mareweb/components/camera.hpp"
#include "mareweb/components/transform.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"
#include "squint/quantity.hpp"

namespace mareweb {
using namespace squint;

constexpr squint::duration DEFAULT_FIXED_TIME_STEP = units::seconds(1.0F / 60.0F);

struct renderer_properties {
  uint32_t width;
  uint32_t height;
  std::string title;
  bool fullscreen = false;
  bool resizable = true;
  wgpu::PresentMode present_mode = wgpu::PresentMode::Fifo; // Fifo is equivalent to VSync
  uint32_t sample_count = 1;                                // MSAA sample count
  wgpu::Color clear_color = {0.0F, 0.0F, 0.0F, 1.0F};
  squint::duration fixed_time_step = DEFAULT_FIXED_TIME_STEP;
};

template <typename T> class renderer_render_system : public render_system<T> {
public:
  void render(const squint::duration &dt, T &rend) override {
    if (rend.is_disabled()) {
      return;
    }
    rend.begin_frame();
    for (const auto &child : rend.get_children()) {
      child->render(dt);
    }
    rend.end_frame();
  }
};

template <typename T> class renderer_physics_system : public physics_system<T> {
public:
  void update(const squint::duration &dt, T &rend) override {
    if (rend.is_disabled()) {
      return;
    }
    for (const auto &child : rend.get_children()) {
      child->update(dt);
    }
  }
};

template <typename T> class renderer_controls_system : public controls_system<T> {
public:
  auto on_key(const key_event &event, T &rend) -> bool override {
    if (rend.is_disabled()) {
      return false;
    }
    for (auto it = rend.get_children().rbegin(); it != rend.get_children().rend(); ++it) {
      if (it->get()->on_key(event)) {
        return true;
      }
    }
    return false;
  }

  auto on_mouse_button(const mouse_button_event &event, T &rend) -> bool override {
    if (rend.is_disabled()) {
      return false;
    }
    for (auto it = rend.get_children().rbegin(); it != rend.get_children().rend(); ++it) {
      if (it->get()->on_mouse_button(event)) {
        return true;
      }
    }
    return false;
  }

  auto on_mouse_move(const mouse_move_event &event, T &rend) -> bool override {
    if (rend.is_disabled()) {
      return false;
    }
    for (auto it = rend.get_children().rbegin(); it != rend.get_children().rend(); ++it) {
      if (it->get()->on_mouse_move(event)) {
        return true;
      }
    }
    return false;
  }

  auto on_mouse_wheel(const mouse_scroll_event &event, T &rend) -> bool override {
    if (rend.is_disabled()) {
      return false;
    }
    for (auto it = rend.get_children().rbegin(); it != rend.get_children().rend(); ++it) {
      if (it->get()->on_mouse_wheel(event)) {
        return true;
      }
    }
    return false;
  }

  auto on_resize(const window_resize_event &event, T &rend) -> bool override {
    if (rend.is_disabled()) {
      return false;
    }
    for (auto it = rend.get_children().rbegin(); it != rend.get_children().rend(); ++it) {
      if (it->get()->on_resize(event)) {
        return true;
      }
    }
    return false;
  }
};

class renderer : public entity<renderer> {
public:
  renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, renderer_properties properties);
  ~renderer() override;

  // Delete copy constructor and copy assignment operator
  renderer(const renderer &) = delete;
  auto operator=(const renderer &) -> renderer & = delete;

  // Default move constructor and move assignment operator
  renderer(renderer &&) noexcept = default;
  auto operator=(renderer &&) noexcept -> renderer & = default;

  void resize(uint32_t new_width, uint32_t new_height);
  void present();

  template <typename MeshType, typename... Args> std::unique_ptr<MeshType> create_mesh(Args &&...args) {
    return std::make_unique<MeshType>(m_device, std::forward<Args>(args)...);
  }
  template <typename MaterialType, typename... Args> std::unique_ptr<MaterialType> create_material(Args &&...args) {
    return std::make_unique<MaterialType>(m_device, m_surface_format, m_properties.sample_count,
                                          std::forward<Args>(args)...);
  }
  void set_fullscreen(bool fullscreen);
  void set_present_mode(wgpu::PresentMode present_mode);
  void set_clear_color(const wgpu::Color &clear_color) { m_clear_color = clear_color; }
  [[nodiscard]] auto get_clear_color() const -> wgpu::Color { return m_clear_color; }
  void begin_frame();
  void end_frame();
  void update_model_view_projection(const transform &model_transform, const camera &cam);

  [[nodiscard]] auto get_window() const -> SDL_Window * { return m_window; }
  [[nodiscard]] auto get_title() const -> const std::string & { return m_properties.title; }
  [[nodiscard]] auto get_properties() const -> const renderer_properties & { return m_properties; }
  [[nodiscard]] auto get_device() const -> wgpu::Device { return m_device; }
  [[nodiscard]] auto get_surface() const -> wgpu::Surface { return m_surface; }
  [[nodiscard]] auto get_surface_format() const -> wgpu::TextureFormat { return m_surface_format; }
  [[nodiscard]] auto get_command_encoder() const -> wgpu::CommandEncoder { return m_command_encoder; }
  [[nodiscard]] auto get_render_pass() const -> wgpu::RenderPassEncoder { return m_render_pass; }
  [[nodiscard]] auto get_current_texture_view() const -> wgpu::TextureView { return m_current_texture_view; }
  [[nodiscard]] auto get_msaa_texture() const -> wgpu::Texture { return m_msaa_texture; }
  [[nodiscard]] auto get_msaa_texture_view() const -> wgpu::TextureView { return m_msaa_texture_view; }
  [[nodiscard]] auto get_depth_texture() const -> wgpu::Texture { return m_depth_texture; }
  [[nodiscard]] auto get_depth_texture_view() const -> wgpu::TextureView { return m_depth_texture_view; }

private:
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
  wgpu::Texture m_depth_texture;
  wgpu::TextureView m_depth_texture_view;

  void configure_surface();
  void create_msaa_texture();
  void create_depth_texture();
};

} // namespace mareweb
#endif // MAREWEB_RENDERER_HPP