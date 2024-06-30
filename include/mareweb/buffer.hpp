#ifndef MAREWEB_BUFFER_HPP
#define MAREWEB_BUFFER_HPP

#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class buffer {
public:
  buffer(wgpu::Device &device, const void *data, size_t size, wgpu::BufferUsage usage);
  ~buffer();

  wgpu::Buffer get_buffer() const { return m_buffer; }
  size_t get_size() const { return m_size; }

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