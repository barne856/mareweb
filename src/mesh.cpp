#include "mareweb/mesh.hpp"
#include <stdexcept>

namespace mareweb {

Mesh::Mesh(wgpu::Device& device, const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
    if (vertices.empty()) {
        throw std::runtime_error("Vertex data is empty");
    }

    m_vertexBuffer = std::make_unique<VertexBuffer>(device, vertices);

    if (!indices.empty()) {
        m_indexBuffer = std::make_unique<IndexBuffer>(device, indices);
    }
}

uint32_t Mesh::getVertexCount() const {
    return static_cast<uint32_t>(m_vertexBuffer->getSize() / (3 * sizeof(float)));  // Assuming 3 floats per vertex
}

uint32_t Mesh::getIndexCount() const {
    return m_indexBuffer ? static_cast<uint32_t>(m_indexBuffer->getSize() / sizeof(uint32_t)) : 0;
}

void Mesh::draw(wgpu::RenderPassEncoder& passEncoder) const {
    passEncoder.SetVertexBuffer(0, m_vertexBuffer->getBuffer(), 0, m_vertexBuffer->getSize());

    if (m_indexBuffer) {
        passEncoder.SetIndexBuffer(m_indexBuffer->getBuffer(), wgpu::IndexFormat::Uint32, 0, m_indexBuffer->getSize());
        passEncoder.DrawIndexed(getIndexCount(), 1, 0, 0, 0);
    } else {
        passEncoder.Draw(getVertexCount(), 1, 0, 0);
    }
}

} // namespace mareweb