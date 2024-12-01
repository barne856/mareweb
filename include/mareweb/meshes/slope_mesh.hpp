#ifndef MAREWEB_SLOPE_MESH_HPP
#define MAREWEB_SLOPE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class slope_mesh : public mesh {
public:
  slope_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(), generate_vertices(size), vertex_layouts::pos3_norm3_tex2()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  static auto generate_vertices(length size) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    vertices.reserve(18); // 6 faces * 3 vertices per triangle (some faces use 6 vertices)

    const float scale = size.value();
    constexpr float SQRT2 = math_constants<float>::sqrt2;

    // Front face
    vertex v1, v2, v3;

    // Front face
    v1 = create_vertex({-0.5f * scale, -0.5f * scale, 0.5f * scale}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f});
    vertices.push_back(v1);

    v2 = create_vertex({0.5f * scale, -0.5f * scale, 0.5f * scale}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f});
    vertices.push_back(v2);

    v3 = create_vertex({0.5f * scale, 0.5f * scale, 0.5f * scale}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f});
    vertices.push_back(v3);

    // Back face
    v1 = create_vertex({0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f});
    vertices.push_back(v1);

    v2 = create_vertex({-0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f});
    vertices.push_back(v2);

    v3 = create_vertex({0.5f * scale, 0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f});
    vertices.push_back(v3);

    // Right face (two triangles)
    v1 = create_vertex({0.5f * scale, -0.5f * scale, 0.5f * scale}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
    vertices.push_back(v1);

    v2 = create_vertex({0.5f * scale, -0.5f * scale, -0.5f * scale}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f});
    vertices.push_back(v2);

    v3 = create_vertex({0.5f * scale, 0.5f * scale, -0.5f * scale}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f});
    vertices.push_back(v3);

    vertices.push_back(v1); // Reuse first vertex
    vertices.push_back(v3); // Reuse third vertex

    v2 = create_vertex({0.5f * scale, 0.5f * scale, 0.5f * scale}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f});
    vertices.push_back(v2);

    // Bottom face (two triangles)
    v1 = create_vertex({0.5f * scale, -0.5f * scale, 0.5f * scale}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f});
    vertices.push_back(v1);

    v2 = create_vertex({-0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f});
    vertices.push_back(v2);

    v3 = create_vertex({0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f});
    vertices.push_back(v3);

    vertices.push_back(v1); // Reuse first vertex

    v2 = create_vertex({-0.5f * scale, -0.5f * scale, 0.5f * scale}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f});
    vertices.push_back(v2);

    v3 = create_vertex({-0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f});
    vertices.push_back(v3);

    // Slope face (two triangles)
    const vec3 slope_normal = {-SQRT2 / 2.0f, SQRT2 / 2.0f, 0.0f};

    v1 = create_vertex({-0.5f * scale, -0.5f * scale, 0.5f * scale}, slope_normal, {0.0f, 0.0f});
    vertices.push_back(v1);

    v2 = create_vertex({0.5f * scale, 0.5f * scale, 0.5f * scale}, slope_normal, {1.0f, 0.0f});
    vertices.push_back(v2);

    v3 = create_vertex({0.5f * scale, 0.5f * scale, -0.5f * scale}, slope_normal, {1.0f, 1.0f});
    vertices.push_back(v3);

    vertices.push_back(v1); // Reuse first vertex
    vertices.push_back(v3); // Reuse third vertex

    v2 = create_vertex({-0.5f * scale, -0.5f * scale, -0.5f * scale}, slope_normal, {0.0f, 1.0f});
    vertices.push_back(v2);

    return vertices;
  }

  static auto create_vertex(const vec3 &position, const vec3 &normal, const vec2 &texcoord) -> vertex {
    vertex v;
    v.position[0] = position[0];
    v.position[1] = position[1];
    v.position[2] = position[2];
    v.normal[0] = normal[0];
    v.normal[1] = normal[1];
    v.normal[2] = normal[2];
    v.texcoord[0] = texcoord[0];
    v.texcoord[1] = texcoord[1];
    return v;
  }
};

} // namespace mareweb
#endif // MAREWEB_SLOPE_MESH_HPP