#ifndef MAREWEB_TRIANGLE_MESH_HPP
#define MAREWEB_TRIANGLE_MESH_HPP

#include "mareweb/mesh.hpp"

namespace mareweb {

using namespace squint;

inline auto calculate_normal(const vec3 &p1, const vec3 &p2, const vec3 &p3) -> vec3 {
  return normalize(cross(p2 - p1, p3 - p1));
}

class triangle_mesh : public mesh {
public:
  triangle_mesh(wgpu::Device &device, const vec3_t<length> &v1, const vec3_t<length> &v2, const vec3_t<length> &v3)
      : mesh(device, get_primitive_state(), generate_vertices(v1, v2, v3), vertex_layouts::pos3_norm3_tex2()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto generate_vertices(const vec3_t<length> &v1, const vec3_t<length> &v2,
                                const vec3_t<length> &v3) -> std::vector<vertex> {
    vec3 p1{v1[0].value(), v1[1].value(), v1[2].value()};
    vec3 p2{v2[0].value(), v2[1].value(), v2[2].value()};
    vec3 p3{v3[0].value(), v3[1].value(), v3[2].value()};

    vec3 normal = calculate_normal(p1, p2, p3);

    std::vector<vertex> vertices(3);

    // First vertex
    vertices[0].position[0] = p1[0];
    vertices[0].position[1] = p1[1];
    vertices[0].position[2] = p1[2];
    vertices[0].normal[0] = normal[0];
    vertices[0].normal[1] = normal[1];
    vertices[0].normal[2] = normal[2];
    vertices[0].texcoord[0] = 0.0f;
    vertices[0].texcoord[1] = 0.0f;

    // Second vertex
    vertices[1].position[0] = p2[0];
    vertices[1].position[1] = p2[1];
    vertices[1].position[2] = p2[2];
    vertices[1].normal[0] = normal[0];
    vertices[1].normal[1] = normal[1];
    vertices[1].normal[2] = normal[2];
    vertices[1].texcoord[0] = 1.0f;
    vertices[1].texcoord[1] = 0.0f;

    // Third vertex
    vertices[2].position[0] = p3[0];
    vertices[2].position[1] = p3[1];
    vertices[2].position[2] = p3[2];
    vertices[2].normal[0] = normal[0];
    vertices[2].normal[1] = normal[1];
    vertices[2].normal[2] = normal[2];
    vertices[2].texcoord[0] = 0.5f;
    vertices[2].texcoord[1] = 1.0f;

    return vertices;
  }
};

} // namespace mareweb
#endif // MAREWEB_TRIANGLE_MESH_HPP