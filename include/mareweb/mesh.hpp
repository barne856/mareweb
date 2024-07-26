#ifndef MAREWEB_MESH_HPP
#define MAREWEB_MESH_HPP

#include "mareweb/buffer.hpp"
#include "webgpu/webgpu_cpp.h"
#include <memory>
#include <vector>

namespace mareweb {

class mesh {
public:
  mesh(wgpu::Device &device, wgpu::PrimitiveTopology topology, const std::vector<float> &vertices,
       const std::vector<uint32_t> &indices = {});

  [[nodiscard]] auto get_vertex_buffer() const -> const vertex_buffer & { return *m_vertex_buffer; }
  [[nodiscard]] auto get_index_buffer() const -> const index_buffer * { return m_index_buffer.get(); }

  [[nodiscard]] auto get_vertex_count() const -> uint32_t;
  [[nodiscard]] auto get_index_count() const -> uint32_t;
  [[nodiscard]] auto get_primitive_topology() const -> wgpu::PrimitiveTopology;

  void draw(wgpu::RenderPassEncoder &pass_encoder) const;

private:
  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<index_buffer> m_index_buffer;
  wgpu::PrimitiveTopology m_topology;
};

} // namespace mareweb

#endif // MAREWEB_MESH_HPP