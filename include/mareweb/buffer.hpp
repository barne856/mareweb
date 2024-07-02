#ifndef MAREWEB_BUFFER_HPP
#define MAREWEB_BUFFER_HPP

#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class buffer {
public:
  buffer(wgpu::Device &device, const void *data, size_t size, wgpu::BufferUsage usage);
  ~buffer();

  // Copy constructor and assignment operator (deleted)
  buffer(const buffer &) = delete;
  auto operator=(const buffer &) -> buffer & = delete;

  // Move constructor
  buffer(buffer&& other) noexcept
    : m_device(std::move(other.m_device)),
      m_buffer(std::exchange(other.m_buffer, nullptr)),
      m_size(std::exchange(other.m_size, 0)) {}

  // Move assignment operator
  auto operator=(buffer&& other) noexcept -> buffer& {
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

  [[nodiscard]] auto get_buffer() const -> wgpu::Buffer { return m_buffer; }
  [[nodiscard]] auto get_size() const -> size_t { return m_size; }

private:
  wgpu::Device m_device;
  wgpu::Buffer m_buffer;
  size_t m_size;
};

class vertex_buffer : public buffer {
public:
  vertex_buffer(wgpu::Device &device, const std::vector<float> &vertices);
};

class index_buffer : public buffer {
public:
  index_buffer(wgpu::Device &device, const std::vector<uint32_t> &indices);
};

} // namespace mareweb

#endif // MAREWEB_BUFFER_HPP