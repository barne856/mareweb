#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

material::material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
                   wgpu::TextureFormat surface_format, uint32_t sample_count)
    : m_device(device) {

  // Create vertex shader
  m_vertex_shader = std::make_unique<shader>(device, vertex_shader_source, wgpu::ShaderStage::Vertex);
  if (!m_vertex_shader) {
    throw std::runtime_error("Failed to create vertex shader");
  }

  // Create fragment shader
  m_fragment_shader = std::make_unique<shader>(device, fragment_shader_source, wgpu::ShaderStage::Fragment);
  if (!m_fragment_shader) {
    throw std::runtime_error("Failed to create fragment shader");
  }

  // Create pipeline
  m_pipeline = std::make_unique<pipeline>(device, *m_vertex_shader, *m_fragment_shader, surface_format, sample_count);
  if (!m_pipeline) {
    throw std::runtime_error("Failed to create pipeline");
  }
}

void material::bind(wgpu::RenderPassEncoder &pass_encoder) const { pass_encoder.SetPipeline(m_pipeline->get_pipeline()); }

} // namespace mareweb