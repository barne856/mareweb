#ifndef MAREWEB_CYLINDER_MESH_HPP
#define MAREWEB_CYLINDER_MESH_HPP

#include "mareweb/mesh.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class cylinder_mesh : public mesh {
public:
  cylinder_mesh(wgpu::Device &device, length radius, length height, float start_angle, float end_angle,
                std::size_t sides)
      : mesh(device, get_primitive_state(), generate_vertices(radius, height, start_angle, end_angle, sides),
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

  static auto generate_vertices(length radius, length height, float start_angle, float end_angle,
                                std::size_t sides) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    float r = radius.value();
    float h = height.value();

    float theta = end_angle - start_angle;
    float dtheta = theta / float(sides);

    // First generate bottom rim vertices
    float angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      vertex vert;
      vert.position[0] = r * std::cos(angle);
      vert.position[1] = r * std::sin(angle);
      vert.position[2] = 0.0f;
      vert.normal[0] = std::cos(angle);
      vert.normal[1] = std::sin(angle);
      vert.normal[2] = 0.0f;
      vert.texcoord[0] = float(i) / float(sides);
      vert.texcoord[1] = 0.0f;
      vertices.push_back(vert);
      angle += dtheta;
    }

    // Then generate top rim vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      vertex vert;
      vert.position[0] = r * std::cos(angle);
      vert.position[1] = r * std::sin(angle);
      vert.position[2] = h;
      vert.normal[0] = std::cos(angle);
      vert.normal[1] = std::sin(angle);
      vert.normal[2] = 0.0f;
      vert.texcoord[0] = float(i) / float(sides);
      vert.texcoord[1] = 1.0f;
      vertices.push_back(vert);
      angle += dtheta;
    }

    // Bottom cap center
    vertex bottom_center;
    bottom_center.position[0] = 0.0f;
    bottom_center.position[1] = 0.0f;
    bottom_center.position[2] = 0.0f;
    bottom_center.normal[0] = 0.0f;
    bottom_center.normal[1] = 0.0f;
    bottom_center.normal[2] = -1.0f;
    bottom_center.texcoord[0] = 0.5f;
    bottom_center.texcoord[1] = 0.5f;
    vertices.push_back(bottom_center);

    // Bottom cap rim vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      vertex vert;
      vert.position[0] = r * std::cos(-angle); // Note the negative angle
      vert.position[1] = r * std::sin(-angle);
      vert.position[2] = 0.0f;
      vert.normal[0] = 0.0f;
      vert.normal[1] = 0.0f;
      vert.normal[2] = -1.0f;
      vert.texcoord[0] = 0.5f + 0.5f * std::cos(-angle);
      vert.texcoord[1] = 0.5f + 0.5f * std::sin(-angle);
      vertices.push_back(vert);
      angle += dtheta;
    }

    // Top cap center
    vertex top_center;
    top_center.position[0] = 0.0f;
    top_center.position[1] = 0.0f;
    top_center.position[2] = h;
    top_center.normal[0] = 0.0f;
    top_center.normal[1] = 0.0f;
    top_center.normal[2] = 1.0f;
    top_center.texcoord[0] = 0.5f;
    top_center.texcoord[1] = 0.5f;
    vertices.push_back(top_center);

    // Top cap rim vertices
    angle = start_angle;
    for (std::size_t i = 0; i <= sides; ++i) {
      vertex vert;
      vert.position[0] = r * std::cos(angle);
      vert.position[1] = r * std::sin(angle);
      vert.position[2] = h;
      vert.normal[0] = 0.0f;
      vert.normal[1] = 0.0f;
      vert.normal[2] = 1.0f;
      vert.texcoord[0] = 0.5f + 0.5f * std::cos(angle);
      vert.texcoord[1] = 0.5f + 0.5f * std::sin(angle);
      vertices.push_back(vert);
      angle += dtheta;
    }

    return vertices;
  }

  static auto generate_indices(std::size_t sides) -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;

    // Wall indices
    for (std::size_t i = 0; i < sides; ++i) {
      indices.push_back(i + 1);         // i+1
      indices.push_back(sides + i + 1); // sides+i+1
      indices.push_back(i);             // i
      indices.push_back(sides + i + 2); // sides+i+2
      indices.push_back(sides + i + 1); // sides+i+1
      indices.push_back(i + 1);         // i+1
    }

    // Bottom cap indices
    uint32_t bottom_center = 2 * (sides + 1);
    uint32_t bottom_start = bottom_center + 1;
    for (std::size_t i = 0; i < sides + 1; ++i) {
      indices.push_back(bottom_center);        // center
      indices.push_back(bottom_start + i + 1); // i+1
      indices.push_back(bottom_start + i);     // i
    }

    // Top cap indices
    uint32_t top_center = bottom_start + sides + 1;
    uint32_t top_start = top_center + 1;
    for (std::size_t i = 0; i < sides + 1; ++i) {
      indices.push_back(top_center);        // center
      indices.push_back(top_start + i + 1); // i+1
      indices.push_back(top_start + i);     // i
    }

    return indices;
  }
};

} // namespace mareweb
#endif // MAREWEB_CYLINDER_MESH_HPP