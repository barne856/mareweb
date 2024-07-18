#ifndef MAREWEB_PIPELINE_HPP
#define MAREWEB_PIPELINE_HPP

#include "mareweb/shader.hpp"
#include "mareweb/buffer.hpp"
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class pipeline {
public:
    pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
             wgpu::TextureFormat surface_format, uint32_t sample_count);

    void setup_uniform_bindings(const std::unordered_map<std::string, std::shared_ptr<uniform_buffer>>& uniform_buffers);
    [[nodiscard]] auto get_pipeline() const -> wgpu::RenderPipeline { return m_pipeline; }
    [[nodiscard]] auto get_bind_group() const -> wgpu::BindGroup { return m_bind_group; }

private:
    void create_pipeline();

    wgpu::Device m_device;
    const shader* m_vertex_shader;
    const shader* m_fragment_shader;
    wgpu::TextureFormat m_surface_format;
    uint32_t m_sample_count;
    wgpu::RenderPipeline m_pipeline;
    wgpu::BindGroupLayout m_bind_group_layout;
    wgpu::BindGroup m_bind_group;
};

} // namespace mareweb

#endif // MAREWEB_PIPELINE_HPP