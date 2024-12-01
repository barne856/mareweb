#ifndef MAREWEB_CIRCLE_MESH_HPP
#define MAREWEB_CIRCLE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"

namespace mareweb {

using namespace squint;

class circle_mesh : public mesh {
public:
  circle_mesh(wgpu::Device &device, length radius, std::size_t segments)
      : mesh(device, get_primitive_state(), generate_vertices(radius, segments), vertex_layouts::pos3_norm3_tex2()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleStrip;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto generate_vertices(length radius, std::size_t segments) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    vertices.reserve((2 * segments + 2));
    float r = radius.value();

    // All vertices share the same normal (facing up Z axis)
    const vec3 normal = {0.0F, 0.0F, 1.0F};

    // Generate alternating center and edge vertices
    for (std::size_t i = 0; i <= segments; i++) {
      float angle = 2.0F * math_constants<float>::pi * float(i) / float(segments);
      float cosA = std::cos(angle);
      float sinA = std::sin(angle);

      // Center vertex
      vertex center_vert;
      center_vert.position[0] = 0.0f;
      center_vert.position[1] = 0.0f;
      center_vert.position[2] = 0.0f;
      center_vert.normal[0] = normal[0];
      center_vert.normal[1] = normal[1];
      center_vert.normal[2] = normal[2];
      center_vert.texcoord[0] = 0.5f;
      center_vert.texcoord[1] = 0.5f;
      vertices.push_back(center_vert);

      // Edge vertex
      vertex edge_vert;
      edge_vert.position[0] = r * cosA;
      edge_vert.position[1] = r * sinA;
      edge_vert.position[2] = 0.0f;
      edge_vert.normal[0] = normal[0];
      edge_vert.normal[1] = normal[1];
      edge_vert.normal[2] = normal[2];
      edge_vert.texcoord[0] = cosA * 0.5f + 0.5f;
      edge_vert.texcoord[1] = sinA * 0.5f + 0.5f;
      vertices.push_back(edge_vert);
    }

    return vertices;
  }
};

} // namespace mareweb
#endif // MAREWEB_CIRCLE_MESH_HPP