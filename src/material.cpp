#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

material::material(wgpu::Device &device, const std::string &vertex_shader_source,
                   const std::string &fragment_shader_source, wgpu::TextureFormat surface_format, uint32_t sample_count,
                   const std::vector<uniform_info> &uniform_infos)
    : m_device(device), m_vertex_shader_source(vertex_shader_source), m_fragment_shader_source(fragment_shader_source),
      m_surface_format(surface_format), m_sample_count(sample_count), m_uniform_infos(uniform_infos) {
  create_shaders();
  create_uniform_buffers();
}

void material::create_shaders() {
  m_vertex_shader = std::make_unique<shader>(m_device, m_vertex_shader_source, wgpu::ShaderStage::Vertex);
  m_fragment_shader = std::make_unique<shader>(m_device, m_fragment_shader_source, wgpu::ShaderStage::Fragment);
}

void material::create_uniform_buffers() {
  for (const auto &info : m_uniform_infos) {
    m_uniform_buffers[info.binding] = std::make_unique<uniform_buffer>(m_device, info.size, info.visibility);
    m_uniform_sizes[info.binding] = info.size;
  }
}

void material::bind(wgpu::RenderPassEncoder &pass_encoder, wgpu::PrimitiveTopology topology) {
  auto &pipeline = get_or_create_pipeline(topology);
  pass_encoder.SetPipeline(pipeline.get_pipeline());
  pass_encoder.SetBindGroup(0, pipeline.get_bind_group());
}

void material::update_uniform(uint32_t binding, const void *data) {
  auto buffer_it = m_uniform_buffers.find(binding);
  auto size_it = m_uniform_sizes.find(binding);

  if (buffer_it == m_uniform_buffers.end() || size_it == m_uniform_sizes.end()) {
    throw std::runtime_error("Uniform buffer not found for binding: " + std::to_string(binding));
  }

  buffer_it->second->update(data, size_it->second);
}

pipeline &material::get_or_create_pipeline(wgpu::PrimitiveTopology topology) {
  auto it = m_pipelines.find(topology);
  if (it == m_pipelines.end()) {
    std::vector<wgpu::BindGroupLayoutEntry> bind_group_layout_entries;
    for (const auto &info : m_uniform_infos) {
      wgpu::BindGroupLayoutEntry entry{};
      entry.binding = info.binding;
      entry.visibility = info.visibility;
      entry.buffer.type = wgpu::BufferBindingType::Uniform;
      entry.buffer.hasDynamicOffset = false;
      entry.buffer.minBindingSize = info.size;
      bind_group_layout_entries.push_back(entry);
    }

    auto new_pipeline = std::make_unique<pipeline>(m_device, *m_vertex_shader, *m_fragment_shader, m_surface_format,
                                                   m_sample_count, bind_group_layout_entries, topology);

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
    bind_group_desc.layout = new_pipeline->get_bind_group_layout();
    bind_group_desc.entryCount = bind_group_entries.size();
    bind_group_desc.entries = bind_group_entries.data();
    new_pipeline->set_bind_group(m_device.CreateBindGroup(&bind_group_desc));

    it = m_pipelines.emplace(topology, std::move(new_pipeline)).first;
  }
  return *it->second;
}

} // namespace mareweb