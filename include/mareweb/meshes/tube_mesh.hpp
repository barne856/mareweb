#ifndef MAREWEB_TUBE_MESH_HPP
#define MAREWEB_TUBE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class tube_mesh : public mesh {
public:
  tube_mesh(wgpu::Device &device, length inner_radius, length thickness, float start_angle, float end_angle,
            std::size_t sides)
      : mesh(device, get_primitive_state(), generate_vertices(inner_radius, thickness, start_angle, end_angle, sides),
             vertex_layouts::pos3_norm3_tex2(), generate_indices(sides)) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto generate_vertices(length inner_radius, length thickness, float start_angle, float end_angle,
                                std::size_t sides) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    const float r_inner = inner_radius.value();
    const float r_outer = r_inner + thickness.value();
    const float theta = end_angle - start_angle;
    const float dtheta = theta / static_cast<float>(sides);
    constexpr float pi = math_constants<float>::pi;

    // Reserve space for vertices (8 sets of rings + 2 end caps)
    vertices.reserve((sides + 1) * 8 + 8);

    // Generate inner surface vertices
    float angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      // Bottom inner vertices
      vertex v1;
      v1.position[0] = r_inner * std::cos(angle);
      v1.position[1] = r_inner * std::sin(angle);
      v1.position[2] = 0.0f;
      v1.normal[0] = -std::cos(angle);
      v1.normal[1] = -std::sin(angle);
      v1.normal[2] = 0.0f;
      v1.texcoord[0] = static_cast<float>(i) / sides;
      v1.texcoord[1] = 0.0f;
      vertices.push_back(v1);

      // Top inner vertices
      vertex v2 = v1;
      v2.position[2] = 1.0f;
      v2.texcoord[1] = 1.0f;
      vertices.push_back(v2);

      angle += dtheta;
    }

    // Generate outer surface vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      // Bottom outer vertices
      vertex v1;
      v1.position[0] = r_outer * std::cos(angle);
      v1.position[1] = r_outer * std::sin(angle);
      v1.position[2] = 0.0f;
      v1.normal[0] = std::cos(angle);
      v1.normal[1] = std::sin(angle);
      v1.normal[2] = 0.0f;
      v1.texcoord[0] = static_cast<float>(i) / sides;
      v1.texcoord[1] = 0.0f;
      vertices.push_back(v1);

      // Top outer vertices
      vertex v2 = v1;
      v2.position[2] = 1.0f;
      v2.texcoord[1] = 1.0f;
      vertices.push_back(v2);

      angle += dtheta;
    }

    // Generate bottom cap vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      // Inner ring
      vertex v1;
      v1.position[0] = r_inner * std::cos(angle);
      v1.position[1] = r_inner * std::sin(angle);
      v1.position[2] = 0.0f;
      v1.normal[0] = 0.0f;
      v1.normal[1] = 0.0f;
      v1.normal[2] = -1.0f;
      v1.texcoord[0] = 0.5f + 0.5f * std::cos(angle);
      v1.texcoord[1] = 0.5f + 0.5f * std::sin(angle);
      vertices.push_back(v1);

      // Outer ring
      vertex v2 = v1;
      v2.position[0] = r_outer * std::cos(angle);
      v2.position[1] = r_outer * std::sin(angle);
      vertices.push_back(v2);

      angle += dtheta;
    }

    // Generate top cap vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      // Inner ring
      vertex v1;
      v1.position[0] = r_inner * std::cos(angle);
      v1.position[1] = r_inner * std::sin(angle);
      v1.position[2] = 1.0f;
      v1.normal[0] = 0.0f;
      v1.normal[1] = 0.0f;
      v1.normal[2] = 1.0f;
      v1.texcoord[0] = 0.5f + 0.5f * std::cos(angle);
      v1.texcoord[1] = 0.5f + 0.5f * std::sin(angle);
      vertices.push_back(v1);

      // Outer ring
      vertex v2 = v1;
      v2.position[0] = r_outer * std::cos(angle);
      v2.position[1] = r_outer * std::sin(angle);
      vertices.push_back(v2);

      angle += dtheta;
    }

    // Add end cap vertices
    // Start cap
    {
      float normal_angle = start_angle - pi / 2.0f;
      vertex v1, v2, v3, v4;

      // Inner bottom
      v1.position[0] = r_inner * std::cos(start_angle);
      v1.position[1] = r_inner * std::sin(start_angle);
      v1.position[2] = 0.0f;
      v1.normal[0] = std::cos(normal_angle);
      v1.normal[1] = std::sin(normal_angle);
      v1.normal[2] = 0.0f;
      v1.texcoord[0] = 0.0f;
      v1.texcoord[1] = 0.0f;
      vertices.push_back(v1);

      // Inner top
      v2 = v1;
      v2.position[2] = 1.0f;
      v2.texcoord[1] = 1.0f;
      vertices.push_back(v2);

      // Outer bottom
      v3.position[0] = r_outer * std::cos(start_angle);
      v3.position[1] = r_outer * std::sin(start_angle);
      v3.position[2] = 0.0f;
      v3.normal[0] = v1.normal[0];
      v3.normal[1] = v1.normal[1];
      v3.normal[2] = v1.normal[2];
      v3.texcoord[0] = 1.0f;
      v3.texcoord[1] = 0.0f;
      vertices.push_back(v3);

      // Outer top
      v4 = v3;
      v4.position[2] = 1.0f;
      v4.texcoord[1] = 1.0f;
      vertices.push_back(v4);
    }

    // End cap
    {
      float normal_angle = end_angle + pi / 2.0f;
      vertex v1, v2, v3, v4;

      // Inner bottom
      v1.position[0] = r_inner * std::cos(end_angle);
      v1.position[1] = r_inner * std::sin(end_angle);
      v1.position[2] = 0.0f;
      v1.normal[0] = std::cos(normal_angle);
      v1.normal[1] = std::sin(normal_angle);
      v1.normal[2] = 0.0f;
      v1.texcoord[0] = 0.0f;
      v1.texcoord[1] = 0.0f;
      vertices.push_back(v1);

      // Inner top
      v2 = v1;
      v2.position[2] = 1.0f;
      v2.texcoord[1] = 1.0f;
      vertices.push_back(v2);

      // Outer bottom
      v3.position[0] = r_outer * std::cos(end_angle);
      v3.position[1] = r_outer * std::sin(end_angle);
      v3.position[2] = 0.0f;
      v3.normal[0] = v1.normal[0];
      v3.normal[1] = v1.normal[1];
      v3.normal[2] = v1.normal[2];
      v3.texcoord[0] = 1.0f;
      v3.texcoord[1] = 0.0f;
      vertices.push_back(v3);

      // Outer top
      v4 = v3;
      v4.position[2] = 1.0f;
      v4.texcoord[1] = 1.0f;
      vertices.push_back(v4);
    }

    return vertices;
  }

  static auto generate_indices(std::size_t sides) -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;
    indices.reserve(sides * 36); // 6 faces * 2 triangles * 3 vertices per triangle

    // Inner surface
    for (std::size_t i = 0; i < sides; ++i) {
      indices.push_back(2 * i);
      indices.push_back(2 * i + 2);
      indices.push_back(2 * i + 1);
      indices.push_back(2 * i + 1);
      indices.push_back(2 * i + 2);
      indices.push_back(2 * i + 3);
    }

    // Outer surface
    uint32_t outer_start = 2 * (sides + 1);
    for (std::size_t i = 0; i < sides; ++i) {
      indices.push_back(outer_start + 2 * i);
      indices.push_back(outer_start + 2 * i + 1);
      indices.push_back(outer_start + 2 * i + 2);
      indices.push_back(outer_start + 2 * i + 1);
      indices.push_back(outer_start + 2 * i + 3);
      indices.push_back(outer_start + 2 * i + 2);
    }

    // Bottom cap
    uint32_t bottom_start = 4 * (sides + 1);
    for (std::size_t i = 0; i < sides; ++i) {
      indices.push_back(bottom_start + 2 * i);
      indices.push_back(bottom_start + 2 * i + 2);
      indices.push_back(bottom_start + 2 * i + 1);
      indices.push_back(bottom_start + 2 * i + 1);
      indices.push_back(bottom_start + 2 * i + 2);
      indices.push_back(bottom_start + 2 * i + 3);
    }

    // Top cap
    uint32_t top_start = 6 * (sides + 1);
    for (std::size_t i = 0; i < sides; ++i) {
      indices.push_back(top_start + 2 * i);
      indices.push_back(top_start + 2 * i + 1);
      indices.push_back(top_start + 2 * i + 2);
      indices.push_back(top_start + 2 * i + 1);
      indices.push_back(top_start + 2 * i + 3);
      indices.push_back(top_start + 2 * i + 2);
    }

    // End caps
    uint32_t endcaps_start = 8 * (sides + 1);

    // Start cap
    indices.push_back(endcaps_start);     // Inner bottom
    indices.push_back(endcaps_start + 2); // Outer bottom
    indices.push_back(endcaps_start + 1); // Inner top
    indices.push_back(endcaps_start + 1); // Inner top
    indices.push_back(endcaps_start + 2); // Outer bottom
    indices.push_back(endcaps_start + 3); // Outer top

    // End cap
    indices.push_back(endcaps_start + 4); // Inner bottom
    indices.push_back(endcaps_start + 5); // Inner top
    indices.push_back(endcaps_start + 6); // Outer bottom
    indices.push_back(endcaps_start + 5); // Inner top
    indices.push_back(endcaps_start + 7); // Outer top
    indices.push_back(endcaps_start + 6); // Outer bottom

    return indices;
  }
};

} // namespace mareweb
#endif // MAREWEB_TUBE_MESH_HPP