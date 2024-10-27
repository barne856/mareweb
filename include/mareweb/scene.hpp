// scene.hpp
#ifndef MAREWEB_SCENE_HPP
#define MAREWEB_SCENE_HPP

#include "mareweb/components/camera.hpp"
#include "mareweb/renderer.hpp"
#include <memory>

namespace mareweb {

class scene : public renderer, public camera {
public:
  scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, const renderer_properties &properties,
        projection_type type = projection_type::perspective);
};

} // namespace mareweb

#endif // MAREWEB_SCENE_HPP