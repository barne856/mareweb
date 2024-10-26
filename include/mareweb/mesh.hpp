#ifndef MAREWEB_MESH_HPP
#define MAREWEB_MESH_HPP

#include "mareweb/buffer.hpp"
#include "mareweb/vertex_attributes.hpp"
#include "webgpu/webgpu_cpp.h"
#include <memory>
#include <vector>

namespace mareweb {

class mesh {
public:
  // Constructor for fully specified vertex data
  mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<vertex> &vertices,
       const vertex_layout &layout, const std::vector<uint32_t> &indices = {});

  // Legacy constructor for position-only data (for backward compatibility)
  mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<float> &vertices,
       const std::vector<uint32_t> &indices = {});

  [[nodiscard]] auto get_vertex_buffer() const -> const vertex_buffer & { return *m_vertex_buffer; }
  [[nodiscard]] auto get_index_buffer() const -> const index_buffer * { return m_index_buffer.get(); }
  [[nodiscard]] auto get_vertex_layout() const -> const vertex_layout & { return m_vertex_layout; }
  [[nodiscard]] auto get_vertex_count() const -> uint32_t;
  [[nodiscard]] auto get_index_count() const -> uint32_t;
  [[nodiscard]] auto get_primitive_state() const -> const wgpu::PrimitiveState & { return m_primitive_state; }

  void draw(wgpu::RenderPassEncoder &pass_encoder) const;

protected:
  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<index_buffer> m_index_buffer;
  vertex_layout m_vertex_layout;
  wgpu::PrimitiveState m_primitive_state;
};

} // namespace mareweb
#endif // MAREWEB_MESH_HPP