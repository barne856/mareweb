#ifndef MAREWEB_PIPELINE_HPP
#define MAREWEB_PIPELINE_HPP

#include "mareweb/buffer.hpp"
#include "mareweb/shader.hpp"
#include <cstdint>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

struct vertex_state {
  bool has_normals = false;
  bool has_texcoords = false;
  bool has_colors = false;
};

class pipeline {
public:
  pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
           wgpu::TextureFormat surface_format, uint32_t sample_count,
           const std::vector<wgpu::BindGroupLayoutEntry> &bind_group_layout_entries,
           const wgpu::PrimitiveState &primitive_state, const vertex_state &vert_state = {});

  [[nodiscard]] auto get_pipeline() const -> wgpu::RenderPipeline { return m_pipeline; }
  [[nodiscard]] auto get_bind_group_layout() const -> wgpu::BindGroupLayout { return m_bind_group_layout; }
  [[nodiscard]] auto get_bind_group() const -> wgpu::BindGroup { return m_bind_group; }
  void set_bind_group(wgpu::BindGroup bind_group) { m_bind_group = bind_group; }

private:
  wgpu::RenderPipeline m_pipeline;
  wgpu::BindGroupLayout m_bind_group_layout;
  wgpu::BindGroup m_bind_group;

  static auto create_vertex_buffer_layout(const vertex_state &vert_state)
      -> std::pair<std::vector<wgpu::VertexAttribute>, wgpu::VertexBufferLayout>;
};

} // namespace mareweb

#endif // MAREWEB_PIPELINE_HPP