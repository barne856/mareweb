#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

Material::Material(wgpu::Device &device, const std::string &vertexShaderSource, const std::string &fragmentShaderSource,
                   wgpu::TextureFormat surfaceFormat, uint32_t sampleCount)
    : m_device(device) {

  // Create vertex shader
  m_vertexShader = std::make_unique<Shader>(device, vertexShaderSource, wgpu::ShaderStage::Vertex);
  if (!m_vertexShader) {
    throw std::runtime_error("Failed to create vertex shader");
  }

  // Create fragment shader
  m_fragmentShader = std::make_unique<Shader>(device, fragmentShaderSource, wgpu::ShaderStage::Fragment);
  if (!m_fragmentShader) {
    throw std::runtime_error("Failed to create fragment shader");
  }

  // Create pipeline
  m_pipeline = std::make_unique<Pipeline>(device, *m_vertexShader, *m_fragmentShader, surfaceFormat, sampleCount);
  if (!m_pipeline) {
    throw std::runtime_error("Failed to create pipeline");
  }
}

void Material::bind(wgpu::RenderPassEncoder &passEncoder) const { passEncoder.SetPipeline(m_pipeline->getPipeline()); }

} // namespace mareweb