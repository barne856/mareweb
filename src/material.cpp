#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

material::material(wgpu::Device &device, const std::string &vertex_shader_source,
                   const std::string &fragment_shader_source, wgpu::TextureFormat surface_format, uint32_t sample_count,
                   const std::vector<uniform_info> &uniform_infos)
    : m_device(device) {

  m_vertex_shader = std::make_unique<shader>(device, vertex_shader_source, wgpu::ShaderStage::Vertex);
  m_fragment_shader = std::make_unique<shader>(device, fragment_shader_source, wgpu::ShaderStage::Fragment);

  std::vector<wgpu::BindGroupLayoutEntry> bind_group_layout_entries;
  for (const auto &info : uniform_infos) {
    m_uniform_buffers[info.binding] = std::make_unique<uniform_buffer>(device, info.size, info.visibility);
    m_uniform_sizes[info.binding] = info.size;

    wgpu::BindGroupLayoutEntry entry{};
    entry.binding = info.binding;
    entry.visibility = info.visibility;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry.buffer.hasDynamicOffset = false;
    entry.buffer.minBindingSize = info.size;
    bind_group_layout_entries.push_back(entry);
  }

  m_pipeline = std::make_unique<pipeline>(device, *m_vertex_shader, *m_fragment_shader, surface_format, sample_count,
                                          bind_group_layout_entries);

  // Create bind group
  std::vector<wgpu::BindGroupEntry> bind_group_entries;
  for (const auto &[binding, buffer] : m_uniform_buffers) {
    wgpu::BindGroupEntry entry{};
    entry.binding = binding;
    entry.buffer = buffer->get_buffer();
    entry.offset = 0;
    entry.size = buffer->get_size();
    bind_group_entries.push_back(entry);
  }

  wgpu::BindGroupDescriptor bind_group_desc{};
  bind_group_desc.layout = m_pipeline->get_bind_group_layout();
  bind_group_desc.entryCount = bind_group_entries.size();
  bind_group_desc.entries = bind_group_entries.data();
  m_pipeline->set_bind_group(device.CreateBindGroup(&bind_group_desc));
}

void material::bind(wgpu::RenderPassEncoder &pass_encoder) const {
  pass_encoder.SetPipeline(m_pipeline->get_pipeline());
  pass_encoder.SetBindGroup(0, m_pipeline->get_bind_group());
}

void material::update_uniform(uint32_t binding, const void *data) {
  auto buffer_it = m_uniform_buffers.find(binding);
  auto size_it = m_uniform_sizes.find(binding);

  if (buffer_it == m_uniform_buffers.end() || size_it == m_uniform_sizes.end()) {
    throw std::runtime_error("Uniform buffer not found for binding: " + std::to_string(binding));
  }

  buffer_it->second->update(data, size_it->second);
}

} // namespace mareweb