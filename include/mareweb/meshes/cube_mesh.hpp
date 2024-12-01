#ifndef MAREWEB_CUBE_MESH_HPP
#define MAREWEB_CUBE_MESH_HPP

#include "mareweb/mesh.hpp"
#include <array>
#include <cstdint>

namespace mareweb {

using namespace squint;

class cube_mesh : public mesh {
public:
  cube_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(), generate_vertices(size), vertex_layouts::pos3_norm3_tex2(),
             generate_indices()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  static void set_vertex(auto &vertices, std::size_t index, const vec3 &position, const vec3 normal,
                         const vec2 texcoord) {
    vertices[index].position[0] = position[0];
    vertices[index].position[1] = position[1];
    vertices[index].position[2] = position[2];
    vertices[index].normal[0] = normal[0];
    vertices[index].normal[1] = normal[1];
    vertices[index].normal[2] = normal[2];
    vertices[index].texcoord[0] = texcoord[0];
    vertices[index].texcoord[1] = texcoord[1];
  }

  static auto generate_vertices(length size) -> std::vector<vertex> {
    float half_size = size.value() / 2.0f;
    std::vector<vertex> vertices(24); // 6 faces * 4 vertices per face

    // Define the 6 face normals
    std::array<vec3, 6> normals = {
        vec3{0.0f, 0.0f, 1.0f},  // Front
        vec3{1.0f, 0.0f, 0.0f},  // Right
        vec3{0.0f, 0.0f, -1.0f}, // Back
        vec3{-1.0f, 0.0f, 0.0f}, // Left
        vec3{0.0f, 1.0f, 0.0f},  // Top
        vec3{0.0f, -1.0f, 0.0f}  // Bottom
    };

    // Front face
    set_vertex(vertices, 0, vec3{-half_size, -half_size, half_size}, normals[0], vec2{0.0f, 0.0f});
    set_vertex(vertices, 1, vec3{half_size, -half_size, half_size}, normals[0], vec2{1.0f, 0.0f});
    set_vertex(vertices, 2, vec3{half_size, half_size, half_size}, normals[0], vec2{1.0f, 1.0f});
    set_vertex(vertices, 3, vec3{-half_size, half_size, half_size}, normals[0], vec2{0.0f, 1.0f});

    // Right face
    set_vertex(vertices, 4, vec3{half_size, -half_size, half_size}, normals[1], vec2{0.0f, 0.0f});
    set_vertex(vertices, 5, vec3{half_size, -half_size, -half_size}, normals[1], vec2{1.0f, 0.0f});
    set_vertex(vertices, 6, vec3{half_size, half_size, -half_size}, normals[1], vec2{1.0f, 1.0f});
    set_vertex(vertices, 7, vec3{half_size, half_size, half_size}, normals[1], vec2{0.0f, 1.0f});

    // Back face
    set_vertex(vertices, 8, vec3{half_size, -half_size, -half_size}, normals[2], vec2{0.0f, 0.0f});
    set_vertex(vertices, 9, vec3{-half_size, -half_size, -half_size}, normals[2], vec2{1.0f, 0.0f});
    set_vertex(vertices, 10, vec3{-half_size, half_size, -half_size}, normals[2], vec2{1.0f, 1.0f});
    set_vertex(vertices, 11, vec3{half_size, half_size, -half_size}, normals[2], vec2{0.0f, 1.0f});

    // Left face
    set_vertex(vertices, 12, vec3{-half_size, -half_size, -half_size}, normals[3], vec2{0.0f, 0.0f});
    set_vertex(vertices, 13, vec3{-half_size, -half_size, half_size}, normals[3], vec2{1.0f, 0.0f});
    set_vertex(vertices, 14, vec3{-half_size, half_size, half_size}, normals[3], vec2{1.0f, 1.0f});
    set_vertex(vertices, 15, vec3{-half_size, half_size, -half_size}, normals[3], vec2{0.0f, 1.0f});

    // Top face
    set_vertex(vertices, 16, vec3{-half_size, half_size, half_size}, normals[4], vec2{0.0f, 0.0f});
    set_vertex(vertices, 17, vec3{half_size, half_size, half_size}, normals[4], vec2{1.0f, 0.0f});
    set_vertex(vertices, 18, vec3{half_size, half_size, -half_size}, normals[4], vec2{1.0f, 1.0f});
    set_vertex(vertices, 19, vec3{-half_size, half_size, -half_size}, normals[4], vec2{0.0f, 1.0f});

    // Bottom face
    set_vertex(vertices, 20, vec3{-half_size, -half_size, -half_size}, normals[5], vec2{0.0f, 0.0f});
    set_vertex(vertices, 21, vec3{half_size, -half_size, -half_size}, normals[5], vec2{1.0f, 0.0f});
    set_vertex(vertices, 22, vec3{half_size, -half_size, half_size}, normals[5], vec2{1.0f, 1.0f});
    set_vertex(vertices, 23, vec3{-half_size, -half_size, half_size}, normals[5], vec2{0.0f, 1.0f});

    return vertices;
  }

  static auto generate_indices() -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;
    indices.reserve(36); // 6 faces * 2 triangles * 3 vertices

    // For each face
    for (uint32_t i = 0; i < 6; i++) {
      uint32_t base = i * 4;
      // First triangle
      indices.push_back(base);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
      // Second triangle
      indices.push_back(base + 2);
      indices.push_back(base + 3);
      indices.push_back(base);
    }

    return indices;
  }
};

} // namespace mareweb
#endif // MAREWEB_CUBE_MESH_HPP