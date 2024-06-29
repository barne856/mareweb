#include "mareweb/pipeline.hpp"
#include <stdexcept>
#include <sys/types.h>

namespace mareweb {

Pipeline::Pipeline(wgpu::Device& device, const Shader& vertexShader, const Shader& fragmentShader, wgpu::TextureFormat surfaceFormat, uint32_t sampleCount)
    : m_device(device) {
    
    // Describe vertex buffer layout
    wgpu::VertexAttribute positionAttribute{};
    positionAttribute.format = wgpu::VertexFormat::Float32x3;
    positionAttribute.offset = 0;
    positionAttribute.shaderLocation = 0;

    wgpu::VertexBufferLayout vertexBufferLayout{};
    vertexBufferLayout.arrayStride = 3 * sizeof(float); // 3 floats per vertex
    vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
    vertexBufferLayout.attributeCount = 1;
    vertexBufferLayout.attributes = &positionAttribute;

    // Create pipeline layout (empty for now)
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 0;
    layoutDesc.bindGroupLayouts = nullptr;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&layoutDesc);

    // Update the color target state
    wgpu::ColorTargetState colorTarget{};
    colorTarget.format = surfaceFormat;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = fragmentShader.getShaderModule();
    fragmentState.entryPoint = "main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // Create render pipeline
    wgpu::RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.layout = pipelineLayout;
    
    pipelineDesc.vertex.module = vertexShader.getShaderModule();
    pipelineDesc.vertex.entryPoint = "main";
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

    pipelineDesc.fragment = &fragmentState;

    pipelineDesc.depthStencil = nullptr; // No depth or stencil for now

    pipelineDesc.multisample.count = sampleCount;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    m_pipeline = device.CreateRenderPipeline(&pipelineDesc);

    if (!m_pipeline) {
        throw std::runtime_error("Failed to create render pipeline");
    }
}

} // namespace mareweb