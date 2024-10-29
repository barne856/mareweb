#include "mareweb/pipeline.hpp"
#include <stdexcept>

namespace mareweb {

constexpr uint32_t kAllSamplesMask = 0xFFFFFFFF;

pipeline::pipeline(wgpu::Device &device, const shader &vertex_shader, const shader &fragment_shader,
                   wgpu::TextureFormat surface_format, uint32_t sample_count,
                   const std::vector<wgpu::BindGroupLayoutEntry> &bind_group_layout_entries,
                   const wgpu::PrimitiveState &primitive_state, const vertex_state &vert_state) {

  // Create bind group layout
  wgpu::BindGroupLayoutDescriptor bind_group_layout_desc{};
  bind_group_layout_desc.entryCount = static_cast<uint32_t>(bind_group_layout_entries.size());
  bind_group_layout_desc.entries = bind_group_layout_entries.data();
  m_bind_group_layout = device.CreateBindGroupLayout(&bind_group_layout_desc);

  // Create pipeline layout
  wgpu::PipelineLayoutDescriptor pipeline_layout_desc{};
  pipeline_layout_desc.bindGroupLayoutCount = 1;
  pipeline_layout_desc.bindGroupLayouts = &m_bind_group_layout;
  wgpu::PipelineLayout pipeline_layout = device.CreatePipelineLayout(&pipeline_layout_desc);

  // Create vertex buffer layout
  auto [attributes, vertex_buffer_layout] = create_vertex_buffer_layout(vert_state);

  // Setup blend state
  wgpu::BlendState blend{};
  blend.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
  blend.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
  blend.color.operation = wgpu::BlendOperation::Add;
  blend.alpha.srcFactor = wgpu::BlendFactor::One;
  blend.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
  blend.alpha.operation = wgpu::BlendOperation::Add;

  // Create color target state
  wgpu::ColorTargetState color_target{};
  color_target.format = surface_format;
  color_target.blend = &blend;
  color_target.writeMask = wgpu::ColorWriteMask::All;

  // Create fragment state
  wgpu::FragmentState fragment_state{};
  fragment_state.module = fragment_shader.get_shader_module();
  fragment_state.entryPoint = "main";
  fragment_state.targetCount = 1;
  fragment_state.targets = &color_target;

  // Create depth stencil state
  wgpu::DepthStencilState depth_stencil{};
  depth_stencil.format = wgpu::TextureFormat::Depth24Plus;
  depth_stencil.depthWriteEnabled = true;
  depth_stencil.depthCompare = wgpu::CompareFunction::Less;

  // Create render pipeline
  wgpu::RenderPipelineDescriptor pipeline_desc{};
  pipeline_desc.layout = pipeline_layout;

  pipeline_desc.vertex.module = vertex_shader.get_shader_module();
  pipeline_desc.vertex.entryPoint = "main";
  pipeline_desc.vertex.bufferCount = 1;
  pipeline_desc.vertex.buffers = &vertex_buffer_layout;

  pipeline_desc.primitive = primitive_state;
  pipeline_desc.fragment = &fragment_state;
  pipeline_desc.depthStencil = &depth_stencil;

  pipeline_desc.multisample.count = sample_count;
  pipeline_desc.multisample.mask = kAllSamplesMask;
  pipeline_desc.multisample.alphaToCoverageEnabled = false;

  m_pipeline = device.CreateRenderPipeline(&pipeline_desc);

  if (!m_pipeline) {
    throw std::runtime_error("Failed to create render pipeline");
  }
}

auto pipeline::create_vertex_buffer_layout(const vertex_state &vert_state)
    -> std::pair<std::vector<wgpu::VertexAttribute>, wgpu::VertexBufferLayout> {

  std::vector<wgpu::VertexAttribute> attributes;
  uint64_t stride = 3 * sizeof(float); // Start with position
  uint64_t current_offset = 0;

  // Position attribute (always present)
  wgpu::VertexAttribute pos_attr{};
  pos_attr.format = wgpu::VertexFormat::Float32x3;
  pos_attr.offset = current_offset;
  pos_attr.shaderLocation = 0;
  attributes.push_back(pos_attr);
  current_offset += 3 * sizeof(float);

  // Normal attribute (optional)
  if (vert_state.has_normals) {
    wgpu::VertexAttribute normal_attr{};
    normal_attr.format = wgpu::VertexFormat::Float32x3;
    normal_attr.offset = current_offset;
    normal_attr.shaderLocation = 1;
    attributes.push_back(normal_attr);
    current_offset += 3 * sizeof(float);
    stride += 3 * sizeof(float);
  }

  // Texture coordinate attribute (optional)
  if (vert_state.has_texcoords) {
    wgpu::VertexAttribute texcoord_attr{};
    texcoord_attr.format = wgpu::VertexFormat::Float32x2;
    texcoord_attr.offset = current_offset;
    texcoord_attr.shaderLocation = 2;
    attributes.push_back(texcoord_attr);
    stride += 2 * sizeof(float);
  }

  // Create the vertex buffer layout
  wgpu::VertexBufferLayout layout{};
  layout.arrayStride = stride;
  layout.stepMode = wgpu::VertexStepMode::Vertex;
  layout.attributeCount = static_cast<uint32_t>(attributes.size());
  // Note: attributes vector must outlive this function call
  layout.attributes = attributes.data();

  return {std::move(attributes), layout};
}

} // namespace mareweb