#include "mareweb/mesh.hpp"
#include <stdexcept>

namespace mareweb {

mesh::mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<vertex> &vertices,
           const vertex_layout &layout, const std::vector<uint32_t> &indices)
    : m_primitive_state(primitive_state), m_vertex_layout(layout) {

  if (vertices.empty()) {
    throw std::runtime_error("Vertex data is empty");
  }

  // Create vertex buffer
  m_vertex_buffer =
      std::make_unique<vertex_buffer>(device, vertices.data(), vertices.size() * sizeof(vertex), m_vertex_layout);

  // Create index buffer if indices are provided
  if (!indices.empty()) {
    m_index_buffer = std::make_unique<index_buffer>(device, indices);
  }
}

mesh::mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<float> &vertices,
           const std::vector<uint32_t> &indices)
    : m_primitive_state(primitive_state), m_vertex_layout(vertex_layouts::pos3()) {

  if (vertices.empty()) {
    throw std::runtime_error("Vertex data is empty");
  }

  // Create vertex buffer
  m_vertex_buffer =
      std::make_unique<vertex_buffer>(device, vertices.data(), vertices.size() * sizeof(float), m_vertex_layout);

  // Create index buffer if indices are provided
  if (!indices.empty()) {
    m_index_buffer = std::make_unique<index_buffer>(device, indices);
  }
}

auto mesh::get_vertex_count() const -> uint32_t {
  return static_cast<uint32_t>(m_vertex_buffer->get_size() / m_vertex_layout.get_stride());
}

auto mesh::get_index_count() const -> uint32_t {
  return m_index_buffer ? static_cast<uint32_t>(m_index_buffer->get_size() / sizeof(uint32_t)) : 0;
}

void mesh::draw(wgpu::RenderPassEncoder &pass_encoder) const {
  pass_encoder.SetVertexBuffer(0, m_vertex_buffer->get_buffer());

  if (m_index_buffer) {
    pass_encoder.SetIndexBuffer(m_index_buffer->get_buffer(), wgpu::IndexFormat::Uint32);
    pass_encoder.DrawIndexed(get_index_count());
  } else {
    pass_encoder.Draw(get_vertex_count());
  }
}

} // namespace mareweb