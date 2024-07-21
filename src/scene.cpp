#include "mareweb/scene.hpp"

namespace mareweb {

scene::scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, const renderer_properties &properties,
             projection_type type)
    : renderer(device, surface, window, properties) {

  m_camera = std::make_unique<camera>(type);
}

} // namespace mareweb