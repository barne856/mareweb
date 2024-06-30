#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include <memory>
#include <string>
#include <sys/types.h>

namespace mareweb {

class material {
public:
  material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
           wgpu::TextureFormat surface_format, uint32_t sample_count);

  void bind(wgpu::RenderPassEncoder &pass_encoder) const;

private:
  wgpu::Device m_device;
  std::unique_ptr<shader> m_vertex_shader;
  std::unique_ptr<shader> m_fragment_shader;
  std::unique_ptr<pipeline> m_pipeline;
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP