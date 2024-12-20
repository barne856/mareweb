#include "mareweb/scene.hpp"
#include "mareweb/renderer.hpp"

namespace mareweb {

scene::scene(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window, const renderer_properties &properties,
             projection_type type)
    : renderer(device, surface, window, properties), camera(type) {}

} // namespace mareweb