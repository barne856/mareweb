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

void buffer::update(const void *data, size_t size, size_t offset) {
  if (offset + size > m_size) {
    throw std::runtime_error("Update range exceeds buffer size");
  }
  m_device.GetQueue().WriteBuffer(m_buffer, offset, data, size);
}

void buffer::update_regions(const std::vector<std::tuple<const void *, size_t, size_t>> &regions) {
  std::vector<std::tuple<const void *, size_t, size_t>> sorted_regions = regions;
  std::sort(sorted_regions.begin(), sorted_regions.end(),
            [](const auto &a, const auto &b) { return std::get<2>(a) < std::get<2>(b); });

  const void *batch_data = nullptr;
  size_t batch_size = 0;
  size_t batch_offset = 0;
  bool batch_active = false;

  for (const auto &[data, size, offset] : sorted_regions) {
    if (offset + size > m_size) {
      throw std::runtime_error("Update range exceeds buffer size");
    }

    if (!batch_active) {
      batch_data = data;
      batch_size = size;
      batch_offset = offset;
      batch_active = true;
    } else if (offset == batch_offset + batch_size) {
      batch_size += size;
    } else {
      m_device.GetQueue().WriteBuffer(m_buffer, batch_offset, batch_data, batch_size);
      batch_data = data;
      batch_size = size;
      batch_offset = offset;
    }
  }

  if (batch_active) {
    m_device.GetQueue().WriteBuffer(m_buffer, batch_offset, batch_data, batch_size);
  }
}

buffer::~buffer() {
  if (m_buffer) {
    m_buffer.Destroy();
  }
}

vertex_buffer::vertex_buffer(wgpu::Device &device, const void *data, size_t size, const vertex_layout &layout)
    : buffer(device, data, size, wgpu::BufferUsage::Vertex), m_layout(layout) {}

auto vertex_buffer::get_buffer_layout() const -> wgpu::VertexBufferLayout {
  wgpu::VertexBufferLayout layout;
  auto attributes = m_layout.get_wgpu_attributes();
  layout.arrayStride = m_layout.get_stride();
  layout.stepMode = wgpu::VertexStepMode::Vertex;
  layout.attributeCount = static_cast<uint32_t>(attributes.size());
  layout.attributes = attributes.data();
  return layout;
}

index_buffer::index_buffer(wgpu::Device &device, const std::vector<uint32_t> &indices)
    : buffer(device, indices.data(), indices.size() * sizeof(uint32_t), wgpu::BufferUsage::Index) {}

uniform_buffer::uniform_buffer(wgpu::Device &device, size_t size, wgpu::ShaderStage visibility)
    : buffer(device, nullptr, size, wgpu::BufferUsage::Uniform), m_visibility(visibility) {}

storage_buffer::storage_buffer(wgpu::Device &device, const void *data, size_t size)
    : buffer(device, data, size, wgpu::BufferUsage::Storage) {}

instance_buffer::instance_buffer(wgpu::Device &device, const std::vector<transform> &instances)
    : storage_buffer(device, nullptr, instances.size() * sizeof(squint::mat4)), m_transforms(instances) {
  update_transforms(instances);
}

void instance_buffer::update_transforms(const std::vector<transform> &instances) {
  m_transforms = instances;
  std::vector<squint::mat4> transforms;
  transforms.reserve(instances.size());
  for (const auto &t : instances) {
    transforms.push_back(t.get_transformation_matrix());
  }
  buffer::update(transforms.data(), transforms.size() * sizeof(squint::mat4));
}

void instance_buffer::update_transform(size_t index, const transform &t) {
  if (index >= m_transforms.size()) {
    throw std::runtime_error("Instance index out of bounds");
  }
  m_transforms[index] = t;
  auto t_matrix = t.get_transformation_matrix();
  buffer::update(&t_matrix, sizeof(squint::mat4), index * sizeof(squint::mat4));
}

void instance_buffer::update_transforms(const std::vector<std::pair<size_t, transform>> &updates) {
  std::vector<std::tuple<const void *, size_t, size_t>> regions;
  regions.reserve(updates.size());

  for (const auto &[index, t] : updates) {
    if (index >= m_transforms.size()) {
      throw std::runtime_error("Instance index out of bounds");
    }
    m_transforms[index] = t;
    auto t_matrix = t.get_transformation_matrix();
    regions.emplace_back(&t_matrix, sizeof(squint::mat4), index * sizeof(squint::mat4));
  }

  buffer::update_regions(regions);
}

auto instance_buffer::get_instance_count() const -> uint32_t { return static_cast<uint32_t>(m_transforms.size()); }

auto instance_buffer::get_transforms() const -> const std::vector<transform> & { return m_transforms; }

auto instance_buffer::get_transform(size_t index) const -> const transform & {
  if (index >= m_transforms.size()) {
    throw std::runtime_error("Instance index out of bounds");
  }
  return m_transforms[index];
}

} // namespace mareweb