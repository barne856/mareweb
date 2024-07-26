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

struct pipeline_key {
  wgpu::PrimitiveTopology topology;
  wgpu::IndexFormat strip_index_format;
  wgpu::FrontFace front_face;
  wgpu::CullMode cull_mode;

  bool operator==(const pipeline_key& other) const {
    return topology == other.topology &&
           strip_index_format == other.strip_index_format &&
           front_face == other.front_face &&
           cull_mode == other.cull_mode;
  }
};

struct pipeline_key_hash {
  std::size_t operator()(const pipeline_key& k) const {
    std::size_t h1 = std::hash<int>()(static_cast<int>(k.topology));
    std::size_t h2 = std::hash<int>()(static_cast<int>(k.strip_index_format));
    std::size_t h3 = std::hash<int>()(static_cast<int>(k.front_face));
    std::size_t h4 = std::hash<int>()(static_cast<int>(k.cull_mode));
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
  }
};

class material {
public:
  material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
           wgpu::TextureFormat surface_format, uint32_t sample_count, const std::vector<uniform_info> &uniform_infos);

  virtual void bind(wgpu::RenderPassEncoder &pass_encoder, const wgpu::PrimitiveState &primitive_state);
  void update_uniform(uint32_t binding, const void *data);

protected:
  wgpu::Device m_device;
  std::string m_vertex_shader_source;
  std::string m_fragment_shader_source;
  wgpu::TextureFormat m_surface_format;
  uint32_t m_sample_count;
  std::vector<uniform_info> m_uniform_infos;

  std::unique_ptr<shader> m_vertex_shader;
  std::unique_ptr<shader> m_fragment_shader;
  std::unordered_map<pipeline_key, std::unique_ptr<pipeline>, pipeline_key_hash> m_pipelines;
  std::unordered_map<uint32_t, std::unique_ptr<uniform_buffer>> m_uniform_buffers;
  std::unordered_map<uint32_t, size_t> m_uniform_sizes;

  void create_shaders();
  void create_uniform_buffers();
  pipeline &get_or_create_pipeline(const wgpu::PrimitiveState &primitive_state);
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP