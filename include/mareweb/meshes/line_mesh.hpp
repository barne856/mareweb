#ifndef MAREWEB_LINE_MESH_HPP
#define MAREWEB_LINE_MESH_HPP

#include "mareweb/mesh.hpp"

namespace mareweb {

using namespace squint;

class line_mesh : public mesh {
public:
  // Single-pixel-width line constructor
  line_mesh(wgpu::Device &device)
      : mesh(device, get_line_primitive_state(), generate_line_vertices(), vertex_layouts::pos3_norm3_tex2()) {}

  // Thick line constructor
  line_mesh(wgpu::Device &device, float thickness)
      : mesh(device, get_triangle_primitive_state(), generate_thick_line_vertices(thickness),
             vertex_layouts::pos3_norm3_tex2()) {}

private:
  static auto get_line_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::LineList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto get_triangle_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  // Generate vertices for single-pixel-width line
  static auto generate_line_vertices() -> std::vector<vertex> {
    std::vector<vertex> vertices(2);

    // Start point (-0.5, 0.0)
    vertices[0].position[0] = -0.5f;
    vertices[0].position[1] = 0.0f;
    vertices[0].position[2] = 0.0f;
    vertices[0].normal[0] = 0.0f;
    vertices[0].normal[1] = 0.0f;
    vertices[0].normal[2] = 1.0f;
    vertices[0].texcoord[0] = 0.0f;
    vertices[0].texcoord[1] = 0.0f;

    // End point (0.5, 0.0)
    vertices[1].position[0] = 0.5f;
    vertices[1].position[1] = 0.0f;
    vertices[1].position[2] = 0.0f;
    vertices[1].normal[0] = 0.0f;
    vertices[1].normal[1] = 0.0f;
    vertices[1].normal[2] = 1.0f;
    vertices[1].texcoord[0] = 1.0f;
    vertices[1].texcoord[1] = 0.0f;

    return vertices;
  }

  // Generate vertices for thick line
  static auto generate_thick_line_vertices(float thickness) -> std::vector<vertex> {
    std::vector<vertex> vertices(6);
    float half_thickness = thickness * 0.5f;

    // First triangle
    vertices[0].position[0] = -0.5f; // Bottom-left
    vertices[0].position[1] = -half_thickness;
    vertices[0].position[2] = 0.0f;

    vertices[1].position[0] = 0.5f; // Bottom-right
    vertices[1].position[1] = -half_thickness;
    vertices[1].position[2] = 0.0f;

    vertices[2].position[0] = 0.5f; // Top-right
    vertices[2].position[1] = half_thickness;
    vertices[2].position[2] = 0.0f;

    // Second triangle
    vertices[3].position[0] = 0.5f; // Top-right (duplicated)
    vertices[3].position[1] = half_thickness;
    vertices[3].position[2] = 0.0f;

    vertices[4].position[0] = -0.5f; // Top-left
    vertices[4].position[1] = half_thickness;
    vertices[4].position[2] = 0.0f;

    vertices[5].position[0] = -0.5f; // Bottom-left (duplicated)
    vertices[5].position[1] = -half_thickness;
    vertices[5].position[2] = 0.0f;

    // Set normals and texture coordinates for all vertices
    for (auto &v : vertices) {
      v.normal[0] = 0.0f;
      v.normal[1] = 0.0f;
      v.normal[2] = 1.0f;
      // Texture coordinates could be added if needed
      v.texcoord[0] = 0.0f;
      v.texcoord[1] = 0.0f;
    }

    return vertices;
  }
};

} // namespace mareweb
#endif // MAREWEB_LINE_MESH_HPP