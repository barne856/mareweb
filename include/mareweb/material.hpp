#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/buffer.hpp"
#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include <memory>
#include <string>
#include <sys/types.h>

namespace mareweb {

struct uniform_info {
  uint32_t binding;
  size_t size;
  wgpu::ShaderStage visibility;
};

class material {
public:
  material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
           wgpu::TextureFormat surface_format, uint32_t sample_count, const std::vector<uniform_info> &uniform_infos);

  void bind(wgpu::RenderPassEncoder &pass_encoder, wgpu::PrimitiveTopology topology);
  void update_uniform(uint32_t binding, const void *data);

private:
  wgpu::Device m_device;
  std::string m_vertex_shader_source;
  std::string m_fragment_shader_source;
  wgpu::TextureFormat m_surface_format;
  uint32_t m_sample_count;
  std::vector<uniform_info> m_uniform_infos;

  std::unique_ptr<shader> m_vertex_shader;
  std::unique_ptr<shader> m_fragment_shader;
  std::unordered_map<wgpu::PrimitiveTopology, std::unique_ptr<pipeline>> m_pipelines;
  std::unordered_map<uint32_t, std::unique_ptr<uniform_buffer>> m_uniform_buffers;
  std::unordered_map<uint32_t, size_t> m_uniform_sizes;

  void create_shaders();
  void create_uniform_buffers();
  pipeline &get_or_create_pipeline(wgpu::PrimitiveTopology topology);
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP