#include "mareweb/shader.hpp"

namespace mareweb {

shader::shader(wgpu::Device &device, const std::string &source, wgpu::ShaderStage stage) : m_device(device) {
  wgpu::ShaderModuleWGSLDescriptor wgsl_desc{};
  wgsl_desc.code = source.c_str();

  wgpu::ShaderModuleDescriptor desc{};
  desc.nextInChain = &wgsl_desc;

  m_shader_module = device.CreateShaderModule(&desc);
}

} // namespace mareweb