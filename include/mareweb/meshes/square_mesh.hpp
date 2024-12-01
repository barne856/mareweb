#ifndef MAREWEB_SQUARE_MESH_HPP
#define MAREWEB_SQUARE_MESH_HPP

#include "mareweb/mesh.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class square_mesh : public mesh {
public:
  square_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(), generate_vertices(size), vertex_layouts::pos3_norm3_tex2(),
             generate_indices()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto generate_vertices(length size) -> std::vector<vertex> {
    float half_size = size.value() / 2.0f;
    std::vector<vertex> vertices(4);

    // All vertices share the same normal (facing up Z axis)
    const vec3 normal = {0.0f, 0.0f, 1.0f};

    // Bottom-left
    vertices[0].position[0] = -half_size;
    vertices[0].position[1] = -half_size;
    vertices[0].position[2] = 0.0f;
    vertices[0].normal[0] = normal[0];
    vertices[0].normal[1] = normal[1];
    vertices[0].normal[2] = normal[2];
    vertices[0].texcoord[0] = 0.0f;
    vertices[0].texcoord[1] = 0.0f;

    // Bottom-right
    vertices[1].position[0] = half_size;
    vertices[1].position[1] = -half_size;
    vertices[1].position[2] = 0.0f;
    vertices[1].normal[0] = normal[0];
    vertices[1].normal[1] = normal[1];
    vertices[1].normal[2] = normal[2];
    vertices[1].texcoord[0] = 1.0f;
    vertices[1].texcoord[1] = 0.0f;

    // Top-right
    vertices[2].position[0] = half_size;
    vertices[2].position[1] = half_size;
    vertices[2].position[2] = 0.0f;
    vertices[2].normal[0] = normal[0];
    vertices[2].normal[1] = normal[1];
    vertices[2].normal[2] = normal[2];
    vertices[2].texcoord[0] = 1.0f;
    vertices[2].texcoord[1] = 1.0f;

    // Top-left
    vertices[3].position[0] = -half_size;
    vertices[3].position[1] = half_size;
    vertices[3].position[2] = 0.0f;
    vertices[3].normal[0] = normal[0];
    vertices[3].normal[1] = normal[1];
    vertices[3].normal[2] = normal[2];
    vertices[3].texcoord[0] = 0.0f;
    vertices[3].texcoord[1] = 1.0f;

    return vertices;
  }

  static auto generate_indices() -> std::vector<uint32_t> { return {0, 1, 2, 2, 3, 0}; }
};

} // namespace mareweb
#endif // MAREWEB_SQUARE_MESH_HPP