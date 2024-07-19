#include "mareweb/buffer.hpp"
#include <stdexcept>

namespace mareweb {

buffer::buffer(wgpu::Device &device, const void *data, size_t size, wgpu::BufferUsage usage)
    : m_device(device), m_size(size) {
  wgpu::BufferDescriptor desc{};
  desc.size = size;
  desc.usage = usage | wgpu::BufferUsage::CopyDst;
  desc.mappedAtCreation = false;

  m_buffer = device.CreateBuffer(&desc);
  if (data != nullptr) {
    device.GetQueue().WriteBuffer(m_buffer, 0, data, size);
  }
}

void buffer::update(const void *data, size_t size) {
  if (size > m_size) {
    throw std::runtime_error("Update size exceeds buffer size");
  }
  m_device.GetQueue().WriteBuffer(m_buffer, 0, data, size);
}

buffer::~buffer() {
  if (m_buffer) {
    m_buffer.Destroy();
  }
}

vertex_buffer::vertex_buffer(wgpu::Device &device, const std::vector<float> &vertices)
    : buffer(device, vertices.data(), vertices.size() * sizeof(float), wgpu::BufferUsage::Vertex) {}

index_buffer::index_buffer(wgpu::Device &device, const std::vector<uint32_t> &indices)
    : buffer(device, indices.data(), indices.size() * sizeof(uint32_t), wgpu::BufferUsage::Index) {}

uniform_buffer::uniform_buffer(wgpu::Device &device, size_t size, wgpu::ShaderStage visibility)
    : buffer(device, nullptr, size, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst), m_visibility(visibility) {
}

} // namespace mareweb