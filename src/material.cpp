#include "mareweb/material.hpp"
#include <stdexcept>

namespace mareweb {

material::material(wgpu::Device &device, const std::string &vertex_shader_source,
                   const std::string &fragment_shader_source, wgpu::TextureFormat surface_format, uint32_t sample_count,
                   const std::vector<binding_resource> &bindings, const vertex_requirements &requirements)
    : m_device(device), m_vertex_shader_source(vertex_shader_source), m_fragment_shader_source(fragment_shader_source),
      m_surface_format(surface_format), m_sample_count(sample_count), m_bindings(bindings),
      m_requirements(requirements) { // Initialize vertex state
  create_shaders();
  create_buffers();
}

void material::bind(wgpu::RenderPassEncoder &pass_encoder, const wgpu::PrimitiveState &primitive_state,
                    const vertex_state &mesh_vertex_state) {
  if (!m_requirements.is_satisfied_by(mesh_vertex_state)) {
    throw std::runtime_error("Mesh does not satisfy material vertex requirements");
  }
  // Use the mesh's vertex state instead of our own
  auto &pipeline = get_or_create_pipeline(primitive_state, mesh_vertex_state);
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

void material::update_texture(uint32_t binding, wgpu::TextureView texture_view) {
  // Find the binding in m_bindings and update its texture view
  for (auto &bind_resource : m_bindings) {
    if (std::holds_alternative<texture_binding>(bind_resource)) {
      auto &tex_binding = std::get<texture_binding>(bind_resource);
      if (tex_binding.binding == binding) {
        tex_binding.texture_view = texture_view;
        return;
      }
    }
  }
  throw std::runtime_error("Texture binding not found: " + std::to_string(binding));
}

void material::update_sampler(uint32_t binding, wgpu::Sampler sampler) {
  // Find the binding in m_bindings and update its sampler
  for (auto &bind_resource : m_bindings) {
    if (std::holds_alternative<sampler_binding>(bind_resource)) {
      auto &samp_binding = std::get<sampler_binding>(bind_resource);
      if (samp_binding.binding == binding) {
        samp_binding.sampler = sampler;
        return;
      }
    }
  }
  throw std::runtime_error("Sampler binding not found: " + std::to_string(binding));
}

void material::update_instance_buffer(wgpu::Buffer buffer, size_t size) {
  for (auto &binding : m_bindings) {
    if (std::holds_alternative<storage_binding>(binding)) {
      auto &instance_binding = std::get<storage_binding>(binding);
      instance_binding.buffer = buffer;
      instance_binding.size = size;
    }
  }
}

void material::create_shaders() {
  m_vertex_shader = std::make_unique<shader>(m_device, m_vertex_shader_source, wgpu::ShaderStage::Vertex);
  m_fragment_shader = std::make_unique<shader>(m_device, m_fragment_shader_source, wgpu::ShaderStage::Fragment);
}

void material::create_buffers() {
  for (const auto &binding : m_bindings) {
    if (std::holds_alternative<uniform_binding>(binding)) {
      const auto &uniform = std::get<uniform_binding>(binding);
      m_uniform_buffers[uniform.binding] = std::make_unique<uniform_buffer>(m_device, uniform.size, uniform.visibility);
      m_uniform_sizes[uniform.binding] = uniform.size;
    }
  }
}

auto material::create_bind_group_layout_entries() const -> std::vector<wgpu::BindGroupLayoutEntry> {
  std::vector<wgpu::BindGroupLayoutEntry> entries;
  entries.reserve(m_bindings.size());

  for (const auto &binding : m_bindings) {
    std::visit(
        [&entries](const auto &b) {
          wgpu::BindGroupLayoutEntry entry{};
          entry.binding = b.binding;
          entry.visibility = b.visibility;

          using T = std::decay_t<decltype(b)>;
          if constexpr (std::is_same_v<T, uniform_binding>) {
            entry.buffer.type = wgpu::BufferBindingType::Uniform;
            entry.buffer.hasDynamicOffset = false;
            entry.buffer.minBindingSize = b.size;
          } else if constexpr (std::is_same_v<T, storage_binding>) {
            entry.buffer.type = b.type;
            entry.buffer.hasDynamicOffset = false;
            entry.buffer.minBindingSize = b.size;
          } else if constexpr (std::is_same_v<T, texture_binding>) {
            entry.texture.sampleType = b.sample_type;
            entry.texture.viewDimension = b.view_dimension;
            entry.texture.multisampled = false;
          } else if constexpr (std::is_same_v<T, sampler_binding>) {
            entry.sampler.type = b.type;
          }
          entries.push_back(entry);
        },
        binding);
  }

  return entries;
}

auto material::create_bind_group_entries() const -> std::vector<wgpu::BindGroupEntry> {
  std::vector<wgpu::BindGroupEntry> entries;
  entries.reserve(m_bindings.size());

  for (const auto &binding : m_bindings) {
    std::visit(
        [&entries, this](const auto &b) {
          wgpu::BindGroupEntry entry{};
          entry.binding = b.binding;

          using T = std::decay_t<decltype(b)>;
          if constexpr (std::is_same_v<T, uniform_binding>) {
            auto it = m_uniform_buffers.find(b.binding);
            if (it != m_uniform_buffers.end()) {
              entry.buffer = it->second->get_buffer();
              entry.offset = 0;
              entry.size = b.size;
            }
          } else if constexpr (std::is_same_v<T, storage_binding>) {
            if (b.buffer) {
              entry.buffer = b.buffer;
              entry.offset = 0;
              entry.size = b.size;
            }
          } else if constexpr (std::is_same_v<T, texture_binding>) {
            entry.textureView = b.texture_view;
          } else if constexpr (std::is_same_v<T, sampler_binding>) {
            entry.sampler = b.sampler;
          }
          entries.push_back(entry);
        },
        binding);
  }

  return entries;
}

auto material::get_or_create_pipeline(const wgpu::PrimitiveState &primitive_state,
                                      const vertex_state &mesh_vertex_state) -> pipeline & {
  pipeline_key key{primitive_state.topology, primitive_state.stripIndexFormat, primitive_state.frontFace,
                   primitive_state.cullMode};

  auto it = m_pipelines.find(key);
  if (it == m_pipelines.end()) {
    // Pass vertex state to pipeline constructor
    auto new_pipeline = std::make_unique<pipeline>(m_device, *m_vertex_shader, *m_fragment_shader, m_surface_format,
                                                   m_sample_count, create_bind_group_layout_entries(), primitive_state,
                                                   mesh_vertex_state); // Pass vertex state

    // Rest of the pipeline creation code...
    auto bind_group_entries = create_bind_group_entries();

    wgpu::BindGroupDescriptor bind_group_desc{};
    bind_group_desc.layout = new_pipeline->get_bind_group_layout();
    bind_group_desc.entryCount = static_cast<uint32_t>(bind_group_entries.size());
    bind_group_desc.entries = bind_group_entries.data();

    new_pipeline->set_bind_group(m_device.CreateBindGroup(&bind_group_desc));

    it = m_pipelines.emplace(key, std::move(new_pipeline)).first;
  }

  return *it->second;
}

} // namespace mareweb