#ifndef MAREWEB_MESH_HPP
#define MAREWEB_MESH_HPP

#include "mareweb/buffer.hpp"
#include <memory>
#include <vector>

namespace mareweb {

class Mesh {
public:
  Mesh(wgpu::Device &device, const std::vector<float> &vertices, const std::vector<uint32_t> &indices = {});

  const VertexBuffer &getVertexBuffer() const { return *m_vertexBuffer; }
  const IndexBuffer *getIndexBuffer() const { return m_indexBuffer.get(); }

  uint32_t getVertexCount() const;
  uint32_t getIndexCount() const;

  void draw(wgpu::RenderPassEncoder &passEncoder) const;

private:
  std::unique_ptr<VertexBuffer> m_vertexBuffer;
  std::unique_ptr<IndexBuffer> m_indexBuffer;
};

} // namespace mareweb

#endif // MAREWEB_MESH_HPP