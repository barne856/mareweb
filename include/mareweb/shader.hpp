#ifndef MAREWEB_SHADER_HPP
#define MAREWEB_SHADER_HPP

#include <string>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class shader {
public:
  shader(wgpu::Device &device, const std::string &source, wgpu::ShaderStage stage);

  [[nodiscard]] auto get_shader_module() const -> wgpu::ShaderModule { return m_shader_module; }

private:
  wgpu::Device m_device;
  wgpu::ShaderModule m_shader_module;
};

} // namespace mareweb

#endif // MAREWEB_SHADER_HPP