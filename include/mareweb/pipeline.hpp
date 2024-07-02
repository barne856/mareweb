#ifndef MAREWEB_PIPELINE_HPP
#define MAREWEB_PIPELINE_HPP

#include "mareweb/shader.hpp"
#include <cstdint>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

class pipeline {
public:
  pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
           wgpu::TextureFormat surface_format, uint32_t sample_count);

  [[nodiscard]] auto get_pipeline() const -> wgpu::RenderPipeline { return m_pipeline; }

private:
  wgpu::Device m_device;
  wgpu::RenderPipeline m_pipeline;
};

} // namespace mareweb

#endif // MAREWEB_PIPELINE_HPP