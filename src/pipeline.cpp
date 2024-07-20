#include "mareweb/pipeline.hpp"
#include <stdexcept>

namespace mareweb {

constexpr uint32_t kAllSamplesMask = 0xFFFFFFFF;

pipeline::pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
                   wgpu::TextureFormat surface_format, uint32_t sample_count)
    : m_device(device), m_vertex_shader(&vertex_shader), m_fragment_shader(&fragment_shader),
      m_surface_format(surface_format), m_sample_count(sample_count) {
    create_pipeline();
}

void pipeline::setup_uniform_bindings(
    const std::unordered_map<uint32_t, std::shared_ptr<uniform_buffer>>& uniform_buffers) {
    std::vector<wgpu::BindGroupLayoutEntry> bind_group_layout_entries;
    std::vector<wgpu::BindGroupEntry> bind_group_entries;

    for (const auto& [binding, buffer] : uniform_buffers) {
        wgpu::BindGroupLayoutEntry layout_entry{};
        layout_entry.binding = binding;
        layout_entry.visibility = buffer->get_visibility();
        layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
        layout_entry.buffer.hasDynamicOffset = false;
        layout_entry.buffer.minBindingSize = buffer->get_size();
        bind_group_layout_entries.push_back(layout_entry);

        wgpu::BindGroupEntry group_entry{};
        group_entry.binding = binding;
        group_entry.buffer = buffer->get_buffer();
        group_entry.offset = 0;
        group_entry.size = buffer->get_size();
        bind_group_entries.push_back(group_entry);
    }

    wgpu::BindGroupLayoutDescriptor bind_group_layout_desc{};
    bind_group_layout_desc.entryCount = bind_group_layout_entries.size();
    bind_group_layout_desc.entries = bind_group_layout_entries.data();
    m_bind_group_layout = m_device.CreateBindGroupLayout(&bind_group_layout_desc);

    wgpu::BindGroupDescriptor bind_group_desc{};
    bind_group_desc.layout = m_bind_group_layout;
    bind_group_desc.entryCount = bind_group_entries.size();
    bind_group_desc.entries = bind_group_entries.data();
    m_bind_group = m_device.CreateBindGroup(&bind_group_desc);

    // Recreate the pipeline with the new bind group layout
    create_pipeline();
}

void pipeline::create_pipeline() {
    // Describe vertex buffer layout
    wgpu::VertexAttribute position_attribute{};
    position_attribute.format = wgpu::VertexFormat::Float32x3;
    position_attribute.offset = 0;
    position_attribute.shaderLocation = 0;

    wgpu::VertexBufferLayout vertex_buffer_layout{};
    vertex_buffer_layout.arrayStride = 3 * sizeof(float); // 3 floats per vertex
    vertex_buffer_layout.stepMode = wgpu::VertexStepMode::Vertex;
    vertex_buffer_layout.attributeCount = 1;
    vertex_buffer_layout.attributes = &position_attribute;

    // Create pipeline layout
    wgpu::PipelineLayoutDescriptor layout_desc{};
    layout_desc.bindGroupLayoutCount = m_bind_group_layout ? 1 : 0;
    layout_desc.bindGroupLayouts = m_bind_group_layout ? &m_bind_group_layout : nullptr;
    wgpu::PipelineLayout pipeline_layout = m_device.CreatePipelineLayout(&layout_desc);

    // Update the color target state
    wgpu::ColorTargetState color_target{};
    color_target.format = m_surface_format;
    color_target.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragment_state{};
    fragment_state.module = m_fragment_shader->get_shader_module();
    fragment_state.entryPoint = "main";
    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target;

    // Create render pipeline
    wgpu::RenderPipelineDescriptor pipeline_desc{};
    pipeline_desc.layout = pipeline_layout;

    pipeline_desc.vertex.module = m_vertex_shader->get_shader_module();
    pipeline_desc.vertex.entryPoint = "main";
    pipeline_desc.vertex.bufferCount = 1;
    pipeline_desc.vertex.buffers = &vertex_buffer_layout;

    pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipeline_desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipeline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipeline_desc.primitive.cullMode = wgpu::CullMode::None;

    pipeline_desc.fragment = &fragment_state;

    pipeline_desc.depthStencil = nullptr; // No depth or stencil for now

    pipeline_desc.multisample.count = m_sample_count;
    pipeline_desc.multisample.mask = kAllSamplesMask;
    pipeline_desc.multisample.alphaToCoverageEnabled = false;

    m_pipeline = m_device.CreateRenderPipeline(&pipeline_desc);

    if (!m_pipeline) {
        throw std::runtime_error("Failed to create render pipeline");
    }
}

} // namespace mareweb