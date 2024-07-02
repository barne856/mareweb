#include "mareweb/mesh.hpp"
#include <stdexcept>

namespace mareweb {

mesh::mesh(wgpu::Device &device, const std::vector<float> &vertices, const std::vector<uint32_t> &indices) {
  if (vertices.empty()) {
    throw std::runtime_error("Vertex data is empty");
  }

  m_vertex_buffer = std::make_unique<vertex_buffer>(device, vertices);

  if (!indices.empty()) {
    m_index_buffer = std::make_unique<index_buffer>(device, indices);
  }
}

auto mesh::get_vertex_count() const -> uint32_t {
  return static_cast<uint32_t>(m_vertex_buffer->get_size() / (3 * sizeof(float))); // Assuming 3 floats per vertex
}

auto mesh::get_index_count() const -> uint32_t {
  return m_index_buffer ? static_cast<uint32_t>(m_index_buffer->get_size() / sizeof(uint32_t)) : 0;
}

void mesh::draw(wgpu::RenderPassEncoder &pass_encoder) const {
  pass_encoder.SetVertexBuffer(0, m_vertex_buffer->get_buffer(), 0, m_vertex_buffer->get_size());

  if (m_index_buffer) {
    pass_encoder.SetIndexBuffer(m_index_buffer->get_buffer(), wgpu::IndexFormat::Uint32, 0, m_index_buffer->get_size());
    pass_encoder.DrawIndexed(get_index_count(), 1, 0, 0, 0);
  } else {
    pass_encoder.Draw(get_vertex_count(), 1, 0, 0);
  }
}

} // namespace mareweb