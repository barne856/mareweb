#include "mareweb/pipeline.hpp"
#include <stdexcept>

namespace mareweb {

constexpr uint32_t kAllSamplesMask = 0xFFFFFFFF;

pipeline::pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
                   wgpu::TextureFormat surface_format, uint32_t sample_count,
                   const std::vector<wgpu::BindGroupLayoutEntry> &bind_group_layout_entries) {
  // Create bind group layout
  wgpu::BindGroupLayoutDescriptor bind_group_layout_desc{};
  bind_group_layout_desc.entryCount = bind_group_layout_entries.size();
  bind_group_layout_desc.entries = bind_group_layout_entries.data();
  m_bind_group_layout = device.CreateBindGroupLayout(&bind_group_layout_desc);

  // Create pipeline layout
  wgpu::PipelineLayoutDescriptor pipeline_layout_desc{};
  pipeline_layout_desc.bindGroupLayoutCount = 1;
  pipeline_layout_desc.bindGroupLayouts = &m_bind_group_layout;
  wgpu::PipelineLayout pipeline_layout = device.CreatePipelineLayout(&pipeline_layout_desc);

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

  // Update the color target state
  wgpu::ColorTargetState color_target{};
  color_target.format = surface_format;
  color_target.writeMask = wgpu::ColorWriteMask::All;

  wgpu::FragmentState fragment_state{};
  fragment_state.module = fragment_shader.get_shader_module();
  fragment_state.entryPoint = "main";
  fragment_state.targetCount = 1;
  fragment_state.targets = &color_target;

  // Create render pipeline
  wgpu::RenderPipelineDescriptor pipeline_desc{};
  pipeline_desc.layout = pipeline_layout;

  pipeline_desc.vertex.module = vertex_shader.get_shader_module();
  pipeline_desc.vertex.entryPoint = "main";
  pipeline_desc.vertex.bufferCount = 1;
  pipeline_desc.vertex.buffers = &vertex_buffer_layout;

  pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
  pipeline_desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
  pipeline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
  pipeline_desc.primitive.cullMode = wgpu::CullMode::None;

  pipeline_desc.fragment = &fragment_state;

  pipeline_desc.depthStencil = nullptr; // No depth or stencil for now

  pipeline_desc.multisample.count = sample_count;
  pipeline_desc.multisample.mask = kAllSamplesMask;
  pipeline_desc.multisample.alphaToCoverageEnabled = false;

  m_pipeline = device.CreateRenderPipeline(&pipeline_desc);

  if (!m_pipeline) {
    throw std::runtime_error("Failed to create render pipeline");
  }
}

} // namespace mareweb