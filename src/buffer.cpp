#include "mareweb/buffer.hpp"

namespace mareweb {

Buffer::Buffer(wgpu::Device &device, const void *data, size_t size, wgpu::BufferUsage usage)
    : m_device(device), m_size(size) {
  wgpu::BufferDescriptor desc{};
  desc.size = size;
  desc.usage = usage | wgpu::BufferUsage::CopyDst;
  desc.mappedAtCreation = false;

  m_buffer = device.CreateBuffer(&desc);
  device.GetQueue().WriteBuffer(m_buffer, 0, data, size);
}

Buffer::~Buffer() { m_buffer.Destroy(); }

VertexBuffer::VertexBuffer(wgpu::Device &device, const std::vector<float> &vertices)
    : Buffer(device, vertices.data(), vertices.size() * sizeof(float), wgpu::BufferUsage::Vertex) {}

IndexBuffer::IndexBuffer(wgpu::Device &device, const std::vector<uint32_t> &indices)
    : Buffer(device, indices.data(), indices.size() * sizeof(uint32_t), wgpu::BufferUsage::Index) {}

} // namespace mareweb