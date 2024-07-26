#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include "mareweb/buffer.hpp"
#include <memory>
#include <string>
#include <sys/types.h>

namespace mareweb {

struct uniform_info {
    uint32_t binding;
    size_t size;
    wgpu::ShaderStage visibility;
};

class material {
public:
    material(wgpu::Device &device, const std::string &vertex_shader_source,
             const std::string &fragment_shader_source, wgpu::TextureFormat surface_format,
             uint32_t sample_count, const std::vector<uniform_info>& uniform_infos);

    void bind(wgpu::RenderPassEncoder &pass_encoder) const;
    void update_uniform(uint32_t binding, const void *data);

private:
    wgpu::Device m_device;
    std::unique_ptr<shader> m_vertex_shader;
    std::unique_ptr<shader> m_fragment_shader;
    std::unique_ptr<pipeline> m_pipeline;
    std::unordered_map<uint32_t, std::unique_ptr<uniform_buffer>> m_uniform_buffers;
    std::unordered_map<uint32_t, size_t> m_uniform_sizes;
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP