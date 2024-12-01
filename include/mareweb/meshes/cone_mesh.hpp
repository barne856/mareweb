#ifndef MAREWEB_CONE_MESH_HPP
#define MAREWEB_CONE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"

namespace mareweb {

using namespace squint;

class cone_mesh : public mesh {
public:
  /**
   * Creates a cone with height = sqrt(3) * radius.
   * Different heights can be achieved by scaling the model in the z direction.
   */
  cone_mesh(wgpu::Device &device, length radius, std::size_t sides)
      : mesh(device, get_primitive_state(), generate_vertices(radius, sides), vertex_layouts::pos3_norm3_tex2()) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  static auto generate_vertices(length radius, std::size_t sides) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    vertices.reserve(sides * 3 + (sides - 2) * 3); // Side triangles + base triangles

    const float r = radius.value();
    constexpr float pi = math_constants<float>::pi;
    constexpr float sqrt2 = math_constants<float>::sqrt2;
    float sqrt3 = std::sqrt(3.0f);
    const float delta_angle = 2.0f * pi / static_cast<float>(sides);
    const float height = r * sqrt3; // cone height = sqrt(3) * radius

    // Generate side faces
    float angle = 0.0f;
    for (std::size_t i = 0; i < sides; ++i) {
      // Calculate vertices
      vec3 p1{r * std::cos(angle), r * std::sin(angle), 0.0f};
      vec3 p2{r * std::cos(angle + delta_angle), r * std::sin(angle + delta_angle), 0.0f};
      vec3 apex{0.0f, 0.0f, height};

      // Calculate normal for the face
      vec3 normal{sqrt2 * std::cos(angle + 0.5f * delta_angle), sqrt2 * std::sin(angle + 0.5f * delta_angle), 1.0f};
      normal = normalize(normal);

      // Calculate texture coordinates
      float u1 = static_cast<float>(i) / sides;
      float u2 = static_cast<float>(i + 1) / sides;

      // First vertex of triangle
      vertex v1;
      v1.position[0] = p1[0];
      v1.position[1] = p1[1];
      v1.position[2] = p1[2];
      v1.normal[0] = normal[0];
      v1.normal[1] = normal[1];
      v1.normal[2] = normal[2];
      v1.texcoord[0] = u1;
      v1.texcoord[1] = 0.0f;
      vertices.push_back(v1);

      // Second vertex of triangle
      vertex v2;
      v2.position[0] = p2[0];
      v2.position[1] = p2[1];
      v2.position[2] = p2[2];
      v2.normal[0] = normal[0];
      v2.normal[1] = normal[1];
      v2.normal[2] = normal[2];
      v2.texcoord[0] = u2;
      v2.texcoord[1] = 0.0f;
      vertices.push_back(v2);

      // Apex vertex of triangle
      vertex v3;
      v3.position[0] = apex[0];
      v3.position[1] = apex[1];
      v3.position[2] = apex[2];
      v3.normal[0] = normal[0];
      v3.normal[1] = normal[1];
      v3.normal[2] = normal[2];
      v3.texcoord[0] = (u1 + u2) * 0.5f;
      v3.texcoord[1] = 1.0f;
      vertices.push_back(v3);

      angle += delta_angle;
    }

    // Generate base (using triangle fan converted to triangle list)
    const vec3 down_normal{0.0f, 0.0f, -1.0f};
    angle = 2.0f * pi - delta_angle;

    for (std::size_t i = 0; i < sides - 2; ++i) {
      // Center vertex
      vertex v1;
      v1.position[0] = r;
      v1.position[1] = 0.0f;
      v1.position[2] = 0.0f;
      v1.normal[0] = down_normal[0];
      v1.normal[1] = down_normal[1];
      v1.normal[2] = down_normal[2];
      v1.texcoord[0] = 0.5f + 0.5f;
      v1.texcoord[1] = 0.5f;
      vertices.push_back(v1);

      // Edge vertex 1
      vertex v2;
      v2.position[0] = r * std::cos(angle);
      v2.position[1] = r * std::sin(angle);
      v2.position[2] = 0.0f;
      v2.normal[0] = down_normal[0];
      v2.normal[1] = down_normal[1];
      v2.normal[2] = down_normal[2];
      v2.texcoord[0] = 0.5f + 0.5f * std::cos(angle);
      v2.texcoord[1] = 0.5f + 0.5f * std::sin(angle);
      vertices.push_back(v2);

      // Edge vertex 2
      vertex v3;
      v3.position[0] = r * std::cos(angle - delta_angle);
      v3.position[1] = r * std::sin(angle - delta_angle);
      v3.position[2] = 0.0f;
      v3.normal[0] = down_normal[0];
      v3.normal[1] = down_normal[1];
      v3.normal[2] = down_normal[2];
      v3.texcoord[0] = 0.5f + 0.5f * std::cos(angle - delta_angle);
      v3.texcoord[1] = 0.5f + 0.5f * std::sin(angle - delta_angle);
      vertices.push_back(v3);

      angle -= delta_angle;
    }

    return vertices;
  }
};

} // namespace mareweb
#endif // MAREWEB_CONE_MESH_HPP