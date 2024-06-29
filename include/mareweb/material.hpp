#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include <memory>
#include <string>
#include <sys/types.h>

namespace mareweb {

class Material {
public:
    Material(wgpu::Device& device, const std::string& vertexShaderSource, const std::string& fragmentShaderSource, wgpu::TextureFormat surfaceFormat, uint32_t sampleCount);

    void bind(wgpu::RenderPassEncoder& passEncoder) const;

private:
    wgpu::Device m_device;
    std::unique_ptr<Shader> m_vertexShader;
    std::unique_ptr<Shader> m_fragmentShader;
    std::unique_ptr<Pipeline> m_pipeline;
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP