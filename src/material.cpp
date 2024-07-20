#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

material::material(wgpu::Device &device, const std::string &vertex_shader_source,
                   const std::string &fragment_shader_source, wgpu::TextureFormat surface_format, uint32_t sample_count)
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

void material::bind(wgpu::RenderPassEncoder &pass_encoder) const {
  pass_encoder.SetPipeline(m_pipeline->get_pipeline());
  pass_encoder.SetBindGroup(0, m_pipeline->get_bind_group(), 0, nullptr);
}

void material::add_uniform_buffer(uint32_t binding_index, std::shared_ptr<uniform_buffer> buffer) {
  m_uniform_buffers[binding_index] = buffer;
  // We need to update the pipeline when adding a new uniform buffer
  m_pipeline->setup_uniform_bindings(m_uniform_buffers);
}

void material::update_uniform_buffer(uint32_t binding_index, const void *data, size_t size) {
  auto it = m_uniform_buffers.find(binding_index);
  if (it == m_uniform_buffers.end()) {
    throw std::runtime_error("Uniform buffer not found: " + std::to_string(binding_index));
  }
  it->second->update(data, size);
}

} // namespace mareweb