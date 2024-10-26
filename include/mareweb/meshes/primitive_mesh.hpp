#ifndef MAREWEB_PRIMITIVE_MESH_HPP
#define MAREWEB_PRIMITIVE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/constants.hpp"
#include "squint/tensor.hpp"
#include <array>
#include <cstdint>

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

class sphere_mesh : public mesh {
public:
  // Icosahedron-based sphere constructor (no texture coordinates)
  sphere_mesh(wgpu::Device &device, length radius, unsigned int recursion_level)
      : mesh(device, get_primitive_state(), generate_icosphere_vertices(radius, recursion_level),
             vertex_layouts::pos3_norm3_tex2()) {}

  // Latitude-longitude based sphere constructor (with texture coordinates)
  sphere_mesh(wgpu::Device &device, length radius, std::size_t n_lats, std::size_t n_lngs)
      : mesh(device, get_primitive_state(), generate_latlong_vertices(radius, n_lats, n_lngs),
             vertex_layouts::pos3_norm3_tex2(), generate_latlong_indices(n_lats, n_lngs)) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  // Generates vertices for icosahedron-based sphere (positions and normals only)
  static auto generate_icosphere_vertices(length radius, unsigned int recursion_level) -> std::vector<vertex> {
    // Constants for icosahedron
    constexpr float X = 0.525731112119133606F;
    constexpr float Z = 0.850650808352039932F;
    constexpr float N = 0.0F;

    // Initial icosahedron vertices
    const ndarr_t<float, 36> verts = {-X, N,  Z, X, N,  Z,  -X, N, -Z, X,  N, -Z, N, Z,  X, N,  Z,  -X,
                                      N,  -Z, X, N, -Z, -X, Z,  X, N,  -Z, X, N,  Z, -X, N, -Z, -X, N};

    // Initial icosahedron indices
    const ndarr_t<unsigned int, 60> indes = {0,  4, 1, 0, 9, 4, 9, 5,  4, 4, 5,  8,  4,  8, 1, 8,  10, 1,  8, 3,
                                             10, 5, 3, 8, 5, 2, 3, 2,  7, 3, 7,  10, 3,  7, 6, 10, 7,  11, 6, 11,
                                             0,  6, 0, 1, 6, 6, 1, 10, 9, 0, 11, 9,  11, 2, 9, 2,  5,  7,  2, 11};

    // Calculate final vertex count
    const std::size_t count = 60 * static_cast<std::size_t>(std::pow(4, recursion_level));
    std::vector<vec3_t<length>> temp_vertices;
    temp_vertices.reserve(count);

    // Subdivide each face of the icosahedron
    for (int i = 0; i < 20; i++) {
      vec3_t<length> v1{length{verts[3 * indes[3 * i]]}, length{verts[3 * indes[3 * i] + 1]},
                        length{verts[3 * indes[3 * i] + 2]}};
      vec3_t<length> v2{length{verts[3 * indes[3 * i + 1]]}, length{verts[3 * indes[3 * i + 1] + 1]},
                        length{verts[3 * indes[3 * i + 1] + 2]}};
      vec3_t<length> v3{length{verts[3 * indes[3 * i + 2]]}, length{verts[3 * indes[3 * i + 2] + 1]},
                        length{verts[3 * indes[3 * i + 2] + 2]}};
      subdivide(v1, v2, v3, recursion_level, temp_vertices);
    }

    // Convert to vertex format with positions and normals
    std::vector<vertex> vertices;
    vertices.reserve(temp_vertices.size());

    for (const auto &pos : temp_vertices) {
      vec3_t<length> scaled_pos = radius * normalize(pos);
      vec3 normal = normalize(pos);

      vertex vert;
      // Position
      vert.position[0] = scaled_pos[0].value();
      vert.position[1] = scaled_pos[1].value();
      vert.position[2] = scaled_pos[2].value();
      // Normal
      vert.normal[0] = normal[0];
      vert.normal[1] = normal[1];
      vert.normal[2] = normal[2];
      // Texture coordinates
      vert.texcoord[0] = 0.0F;
      vert.texcoord[1] = 0.0F;

      vertices.push_back(vert);
    }

    return vertices;
  }

  // Generates vertices for latitude-longitude based sphere (positions, normals, and texture coordinates)
  static auto generate_latlong_vertices(length radius, std::size_t n_lats, std::size_t n_lngs) -> std::vector<vertex> {
    n_lngs = std::max<std::size_t>(3, n_lngs);
    n_lats = std::max<std::size_t>(2, n_lats);

    std::vector<vertex> vertices;
    vertices.reserve((n_lats + 1) * (n_lngs + 1));

    constexpr auto pi = math_constants<float>::pi;
    constexpr auto two_pi = 2.0F * pi;

    angle delta_lat = angle{pi / static_cast<float>(n_lats)};
    angle delta_lng = angle{two_pi / static_cast<float>(n_lngs)};

    for (std::size_t i = 0; i <= n_lats; i++) {
      angle lat = angle{pi / 2.0F} - i * delta_lat;
      length xy = radius * std::cos(lat);
      length z = radius * std::sin(lat);

      for (std::size_t j = 0; j <= n_lngs; j++) {
        angle lng = j * delta_lng;

        vertex vert;

        // Position
        length x = xy * std::cos(lng);
        length y = xy * std::sin(lng);
        vert.position[0] = x.value();
        vert.position[1] = y.value();
        vert.position[2] = z.value();

        // Normal (normalized position)
        vert.normal[0] = (x / radius).value();
        vert.normal[1] = (y / radius).value();
        vert.normal[2] = (z / radius).value();

        // Texture coordinates
        vert.texcoord[0] = 1.0F - static_cast<float>(j) / n_lngs;
        vert.texcoord[1] = 1.0F - static_cast<float>(i) / n_lats;

        vertices.push_back(vert);
      }
    }

    return vertices;
  }

  // Generates indices for latitude-longitude based sphere
  static auto generate_latlong_indices(std::size_t n_lats, std::size_t n_lngs) -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;
    indices.reserve(n_lats * n_lngs * 6);

    for (std::size_t i = 0; i < n_lats; ++i) {
      uint32_t k1 = i * (n_lngs + 1);
      uint32_t k2 = k1 + n_lngs + 1;

      for (std::size_t j = 0; j < n_lngs; ++j, ++k1, ++k2) {
        if (i != 0) {
          indices.push_back(k1);
          indices.push_back(k2);
          indices.push_back(k1 + 1);
        }

        if (i != (n_lats - 1)) {
          indices.push_back(k1 + 1);
          indices.push_back(k2);
          indices.push_back(k2 + 1);
        }
      }
    }
    return indices;
  }

  static void subdivide(const vec3_t<length> &v1, const vec3_t<length> &v2, const vec3_t<length> &v3, int depth,
                        std::vector<vec3_t<length>> &vertices) {
    if (depth == 0) {
      vertices.push_back(v1);
      vertices.push_back(v2);
      vertices.push_back(v3);
      return;
    }

    auto v12 = normalize((v1 + v2) * 0.5F) * length(1);
    auto v23 = normalize((v2 + v3) * 0.5F) * length(1);
    auto v31 = normalize((v3 + v1) * 0.5F) * length(1);

    subdivide(v1, v12, v31, depth - 1, vertices);
    subdivide(v2, v23, v12, depth - 1, vertices);
    subdivide(v3, v31, v23, depth - 1, vertices);
    subdivide(v12, v23, v31, depth - 1, vertices);
  }
};

} // namespace mareweb
#endif // MAREWEB_PRIMITIVE_MESH_HPP