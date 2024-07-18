#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include "mareweb/buffer.hpp"
#include <memory>
#include <string>
#include <sys/types.h>

namespace mareweb {

class material {
public:
  material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
           wgpu::TextureFormat surface_format, uint32_t sample_count);

  void bind(wgpu::RenderPassEncoder &pass_encoder) const;
  void add_uniform_buffer(const std::string &name, std::shared_ptr<uniform_buffer> buffer);
  void update_uniform_buffer(const std::string &name, const void *data, size_t size);

private:
  wgpu::Device m_device;
  std::unique_ptr<shader> m_vertex_shader;
  std::unique_ptr<shader> m_fragment_shader;
  std::unique_ptr<pipeline> m_pipeline;
  std::unordered_map<std::string, std::shared_ptr<uniform_buffer>> m_uniform_buffers;
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP