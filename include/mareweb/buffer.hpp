#ifndef MAREWEB_BUFFER_HPP
#define MAREWEB_BUFFER_HPP

#include <webgpu/webgpu_cpp.h>
#include <vector>

namespace mareweb {

class Buffer {
public:
    Buffer(wgpu::Device& device, const void* data, size_t size, wgpu::BufferUsage usage);
    ~Buffer();

    wgpu::Buffer getBuffer() const { return m_buffer; }
    size_t getSize() const { return m_size; }

private:
    wgpu::Device m_device;
    wgpu::Buffer m_buffer;
    size_t m_size;
};

class VertexBuffer : public Buffer {
public:
    VertexBuffer(wgpu::Device& device, const std::vector<float>& vertices);
};

class IndexBuffer : public Buffer {
public:
    IndexBuffer(wgpu::Device& device, const std::vector<uint32_t>& indices);
};

} // namespace mareweb

#endif // MAREWEB_BUFFER_HPP