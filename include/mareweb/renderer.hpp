#ifndef MAREWEB_RENDERER_HPP
#define MAREWEB_RENDERER_HPP

#include <SDL2/SDL.h>
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

#include "mareweb/components/transform.hpp"
#include "mareweb/entities/camera.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"

namespace mareweb {
using namespace squint;

constexpr units::time DEFAULT_FIXED_TIME_STEP = units::time::seconds(1.0F / 60.0F);

struct renderer_properties {
  uint32_t width;
  uint32_t height;
  std::string title;
  bool fullscreen = false;
  bool resizable = true;
  wgpu::PresentMode present_mode = wgpu::PresentMode::Fifo; // Fifo is equivalent to VSync
  uint32_t sample_count = 1;                                // MSAA sample count
  wgpu::Color clear_color = {0.0F, 0.0F, 0.0F, 1.0F};
  units::time fixed_time_step = DEFAULT_FIXED_TIME_STEP;
};

template <typename T> class renderer_render_system : public render_system<T> {
public:
  void render(const units::time &dt, T &rend) override {
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
  void update(const units::time &dt, T &rend) override {
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

  auto create_mesh(wgpu::PrimitiveTopology topology, const std::vector<float> &vertices,
                   const std::vector<uint32_t> &indices = {}) -> std::unique_ptr<mesh>;
  auto create_material(const std::string &vertex_shader_source, const std::string &fragment_shader_source,
                       const std::vector<uniform_info> &uniform_infos) -> std::unique_ptr<material>;
  void set_fullscreen(bool fullscreen);
  void set_present_mode(wgpu::PresentMode present_mode);
  void set_clear_color(const wgpu::Color &clear_color) { m_clear_color = clear_color; }
  [[nodiscard]] auto get_clear_color() const -> wgpu::Color { return m_clear_color; }
  void begin_frame();
  void end_frame();
  void draw_mesh(const mesh &mesh, material &material);
  void update_model_view_projection(const transform &model_transform, const camera &cam);

  [[nodiscard]] auto get_window() const -> SDL_Window * { return m_window; }
  [[nodiscard]] auto get_title() const -> const std::string & { return m_properties.title; }
  [[nodiscard]] auto get_properties() const -> const renderer_properties & { return m_properties; }

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

  void configure_surface();
  void create_msaa_texture();

  std::shared_ptr<uniform_buffer> m_mvp_buffer;
};

} // namespace mareweb
#endif // MAREWEB_RENDERER_HPP