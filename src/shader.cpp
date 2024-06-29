#include "mareweb/shader.hpp"

namespace mareweb {

Shader::Shader(wgpu::Device& device, const std::string& source, wgpu::ShaderStage stage)
    : m_device(device) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = source.c_str();

    wgpu::ShaderModuleDescriptor desc{};
    desc.nextInChain = &wgslDesc;

    m_shaderModule = device.CreateShaderModule(&desc);
}

} // namespace mareweb