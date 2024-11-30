#ifndef MAREWEB_BUFFER_HPP
#define MAREWEB_BUFFER_HPP

#include "mareweb/components/transform.hpp"
#include "mareweb/vertex_attributes.hpp"
#include <utility>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class buffer {
public:
  buffer(wgpu::Device &device, const void *data, size_t size, wgpu::BufferUsage usage);
  virtual ~buffer();

  // Copy constructor and assignment operator (deleted)
  buffer(const buffer &) = delete;
  auto operator=(const buffer &) -> buffer & = delete;

  // Move constructor
  buffer(buffer &&other) noexcept
      : m_device(std::move(other.m_device)), m_buffer(std::exchange(other.m_buffer, nullptr)),
        m_size(std::exchange(other.m_size, 0)) {}

  // Move assignment operator
  auto operator=(buffer &&other) noexcept -> buffer & {
    if (this != &other) {
      // Clean up current buffer if necessary
      if (m_buffer) {
        m_buffer.Destroy();
      }
      m_device = std::move(other.m_device);
      m_buffer = std::exchange(other.m_buffer, nullptr);
      m_size = std::exchange(other.m_size, 0);
    }
    return *this;
  }

  virtual void update(const void *data, size_t size);
  virtual void update(const void *data, size_t size, size_t offset);
  virtual void update_regions(const std::vector<std::tuple<const void *, size_t, size_t>> &regions);

  [[nodiscard]] virtual auto get_buffer() const -> wgpu::Buffer { return m_buffer; }
  [[nodiscard]] virtual auto get_size() const -> size_t { return m_size; }

protected:
  wgpu::Device m_device;
  wgpu::Buffer m_buffer;
  size_t m_size;
};

class vertex_buffer : public buffer {
public:
  vertex_buffer(wgpu::Device &device, const void *data, size_t size, const vertex_layout &layout);

  [[nodiscard]] auto get_layout() const -> const vertex_layout & { return m_layout; }
  [[nodiscard]] auto get_buffer_layout() const -> wgpu::VertexBufferLayout;

private:
  vertex_layout m_layout;
};

class index_buffer : public buffer {
public:
  index_buffer(wgpu::Device &device, const std::vector<uint32_t> &indices);
};

class uniform_buffer : public buffer {
public:
  uniform_buffer(wgpu::Device &device, size_t size, wgpu::ShaderStage visibility);

  [[nodiscard]] auto get_visibility() const -> wgpu::ShaderStage { return m_visibility; }

private:
  wgpu::ShaderStage m_visibility;
};

class storage_buffer : public buffer {
public:
  storage_buffer(wgpu::Device &device, const void *data, size_t size);
};

class instance_buffer : public storage_buffer {
public:
  instance_buffer(wgpu::Device &device, const std::vector<transform> &instances);

  void update_transforms(const std::vector<transform> &instances);
  void update_transform(size_t index, const transform &t);
  void update_transforms(const std::vector<std::pair<size_t, transform>> &updates);

  [[nodiscard]] auto get_instance_count() const -> uint32_t;
  [[nodiscard]] auto get_transforms() const -> const std::vector<transform> &;
  [[nodiscard]] auto get_transform(size_t index) const -> const transform &;

private:
  std::vector<transform> m_transforms;
};

} // namespace mareweb

#endif // MAREWEB_BUFFER_HPP