#ifndef MAREWEB_PIPELINE_HPP
#define MAREWEB_PIPELINE_HPP

#include <webgpu/webgpu_cpp.h>
#include "mareweb/shader.hpp"

namespace mareweb {

class Pipeline {
public:
    Pipeline(wgpu::Device& device, const Shader& vertexShader, const Shader& fragmentShader);

    wgpu::RenderPipeline getPipeline() const { return m_pipeline; }

private:
    wgpu::Device m_device;
    wgpu::RenderPipeline m_pipeline;
};

} // namespace mareweb

#endif // MAREWEB_PIPELINE_HPP