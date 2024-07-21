// scene.hpp
#ifndef MAREWEB_SCENE_HPP
#define MAREWEB_SCENE_HPP

#include "mareweb/entities/camera.hpp"
#include "mareweb/renderer.hpp"
#include <memory>

namespace mareweb {

class scene : public renderer {
public:
  scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
        const renderer_properties &properties, projection_type type = projection_type::perspective);

  [[nodiscard]] auto get_camera() const -> const camera * { return m_camera.get(); }
  auto get_camera() -> camera * { return m_camera.get(); }

private:
  std::unique_ptr<camera> m_camera;
};

} // namespace mareweb

#endif // MAREWEB_SCENE_HPP