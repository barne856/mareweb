#ifndef MAREWEB_SHADER_HPP
#define MAREWEB_SHADER_HPP

#include <string>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class Shader {
public:
  Shader(wgpu::Device &device, const std::string &source, wgpu::ShaderStage stage);

  wgpu::ShaderModule getShaderModule() const { return m_shaderModule; }

private:
  wgpu::Device m_device;
  wgpu::ShaderModule m_shaderModule;
};

} // namespace mareweb

#endif // MAREWEB_SHADER_HPP