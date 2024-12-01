#ifndef MAREWEB_TORUS_MESH_HPP
#define MAREWEB_TORUS_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class torus_mesh : public mesh {
public:
  torus_mesh(wgpu::Device &device, length outer_radius, length inner_radius, std::size_t n_rings,
             std::size_t n_segments)
      : mesh(device, get_primitive_state(), generate_vertices(outer_radius, inner_radius, n_rings, n_segments),
             vertex_layouts::pos3_norm3_tex2(), generate_indices(n_rings, n_segments)) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  static auto generate_vertices(length outer_radius, length inner_radius, std::size_t n_rings,
                                std::size_t n_segments) -> std::vector<vertex> {
    // Ensure minimum number of rings and segments
    n_rings = std::max<std::size_t>(3, n_rings);
    n_segments = std::max<std::size_t>(3, n_segments);

    std::vector<vertex> vertices;
    vertices.reserve((n_rings + 1) * (n_segments + 1));

    float R = outer_radius.value(); // Major radius (distance from center of tube to center of torus)
    float r = inner_radius.value(); // Minor radius (radius of the tube)

    // Generate vertices
    for (std::size_t i = 0; i <= n_rings; ++i) {
      float u = 2.0F * math_constants<float>::pi * float(i) / float(n_rings);
      float cos_u = std::cos(u);
      float sin_u = std::sin(u);

      for (std::size_t j = 0; j <= n_segments; ++j) {
        float v = 2.0F * math_constants<float>::pi * float(j) / float(n_segments);
        float cos_v = std::cos(v);
        float sin_v = std::sin(v);

        vertex vert;

        // Position
        vert.position[0] = (R + r * cos_v) * cos_u;
        vert.position[1] = (R + r * cos_v) * sin_u;
        vert.position[2] = r * sin_v;

        // Normal
        vert.normal[0] = cos_v * cos_u;
        vert.normal[1] = cos_v * sin_u;
        vert.normal[2] = sin_v;

        // Texture coordinates
        vert.texcoord[0] = float(i) / float(n_rings);
        vert.texcoord[1] = float(j) / float(n_segments);

        vertices.push_back(vert);
      }
    }

    return vertices;
  }

  static auto generate_indices(std::size_t n_rings, std::size_t n_segments) -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;
    indices.reserve(n_rings * n_segments * 6);

    for (std::size_t i = 0; i < n_rings; ++i) {
      for (std::size_t j = 0; j < n_segments; ++j) {
        uint32_t current = i * (n_segments + 1) + j;
        uint32_t next = current + n_segments + 1;

        // First triangle
        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(current + 1);

        // Second triangle
        indices.push_back(current + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
      }
    }

    return indices;
  }
};

} // namespace mareweb
#endif // MAREWEB_TORUS_MESH_HPP