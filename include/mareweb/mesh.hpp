#ifndef MAREWEB_MESH_HPP
#define MAREWEB_MESH_HPP

#include "mareweb/buffer.hpp"
#include <memory>
#include <vector>

namespace mareweb {

class mesh {
public:
  mesh(wgpu::Device &device, const std::vector<float> &vertices, const std::vector<uint32_t> &indices = {});

  const vertex_buffer &get_vertex_buffer() const { return *m_vertex_buffer; }
  const index_buffer *get_index_buffer() const { return m_index_buffer.get(); }

  uint32_t get_vertex_count() const;
  uint32_t get_index_count() const;

  void draw(wgpu::RenderPassEncoder &pass_encoder) const;

private:
  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<index_buffer> m_index_buffer;
};

} // namespace mareweb

#endif // MAREWEB_MESH_HPP