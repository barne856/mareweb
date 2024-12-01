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
             vertex_layouts::pos3_norm3()) {}

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

    // Initial icosahedron indices - Fixed winding order
    const ndarr_t<unsigned int, 60> indes = {
        0, 4, 1,  0, 9, 4,  9, 5, 4,  4, 5, 8,  4, 8, 1,  8, 10, 1,
        8, 3, 10, 5, 3, 8,  5, 2, 3,  2, 7, 3,  7, 10, 3, 7, 6, 10,
        7, 11, 6, 11, 0, 6, 0, 1, 6,  6, 1, 10, 9, 0, 11, 9, 11, 2,
        9, 2, 5,  7, 2, 11
    };

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
      
      // Check if we need to flip the winding order based on the normal direction
      vec3 normal = cross(v2 - v1, v3 - v1).values();
      if (dot(normal, v1).value() < 0) {
        std::swap(v2, v3);
      }

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
      // Ensure consistent winding order
      vec3 normal = cross(v2 - v1, v3 - v1).values();
      if (dot(normal, v1).value() >= 0) {
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
      } else {
        vertices.push_back(v1);
        vertices.push_back(v3);
        vertices.push_back(v2);
      }
      return;
    }

    auto v12 = normalize((v1 + v2) * 0.5F) * length(1);
    auto v23 = normalize((v2 + v3) * 0.5F) * length(1);
    auto v31 = normalize((v3 + v1) * 0.5F) * length(1);

    subdivide(v1, v12, v31, depth - 1, vertices);
    subdivide(v12, v2, v23, depth - 1, vertices);
    subdivide(v31, v23, v3, depth - 1, vertices);
    subdivide(v12, v23, v31, depth - 1, vertices);
  }
};

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

class char_mesh : public mesh {
public:
  char_mesh(wgpu::Device &device, const std::string &text, float thickness = 0.0f)
      : mesh(device, get_primitive_state(), generate_vertices(text, thickness), vertex_layouts::pos3_norm3_tex2(),
             generate_indices(text)) {
    update_text_metrics(text);
  }

  // Getters for text metrics
  auto get_width() const -> uint32_t { return m_width; }
  auto get_lines() const -> uint32_t { return m_lines; }
  auto get_text() const -> const std::string & { return m_text; }

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  void update_text_metrics(const std::string &text) {
    m_text = text;
    m_width = 0;
    m_lines = 1;
    uint32_t current_width = 0;

    for (char c : text) {
      if (c == '\n') {
        m_lines++;
        m_width = std::max(m_width, current_width);
        current_width = 0;
      } else {
        current_width++;
      }
    }
    m_width = std::max(m_width, current_width);
  }

  static const float GRID_POINTS[17];
  static const std::unordered_map<char, std::vector<uint32_t>> ASCII_FONT;

  struct CharacterStroke {
    vec3 start;
    vec3 end;
    vec3 normal;
  };

  static auto generate_stroke(const vec3 &p1, const vec3 &p2, float thickness) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    vertices.reserve(6); // 2 triangles = 6 vertices

    vec3 direction = normalize(p2 - p1);
    vec3 normal = vec3{-direction[1], direction[0], 0.0f}; // perpendicular in 2D
    vec3 offset = normal * (thickness * 0.5f);

    // Bottom left
    vertex v1;
    v1.position[0] = p1[0] - offset[0];
    v1.position[1] = p1[1] - offset[1];
    v1.position[2] = p1[2];
    v1.normal[0] = 0.0f;
    v1.normal[1] = 0.0f;
    v1.normal[2] = 1.0f;
    v1.texcoord[0] = 0.0f;
    v1.texcoord[1] = 0.0f;

    // Top left
    vertex v2;
    v2.position[0] = p1[0] + offset[0];
    v2.position[1] = p1[1] + offset[1];
    v2.position[2] = p1[2];
    v2.normal[0] = 0.0f;
    v2.normal[1] = 0.0f;
    v2.normal[2] = 1.0f;
    v2.texcoord[0] = 0.0f;
    v2.texcoord[1] = 1.0f;

    // Bottom right
    vertex v3;
    v3.position[0] = p2[0] - offset[0];
    v3.position[1] = p2[1] - offset[1];
    v3.position[2] = p2[2];
    v3.normal[0] = 0.0f;
    v3.normal[1] = 0.0f;
    v3.normal[2] = 1.0f;
    v3.texcoord[0] = 1.0f;
    v3.texcoord[1] = 0.0f;

    // Top right
    vertex v4;
    v4.position[0] = p2[0] + offset[0];
    v4.position[1] = p2[1] + offset[1];
    v4.position[2] = p2[2];
    v4.normal[0] = 0.0f;
    v4.normal[1] = 0.0f;
    v4.normal[2] = 1.0f;
    v4.texcoord[0] = 1.0f;
    v4.texcoord[1] = 1.0f;

    // First triangle
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);

    // Second triangle
    vertices.push_back(v2);
    vertices.push_back(v4);
    vertices.push_back(v3);

    return vertices;
  }

  static auto get_character_strokes(char c, vec3 offset) -> std::vector<CharacterStroke> {
    std::vector<CharacterStroke> strokes;
    auto it = ASCII_FONT.find(c);
    if (it == ASCII_FONT.end())
      return strokes;

    const auto &indices = it->second;
    for (size_t i = 0; i < indices.size(); i += 4) {
      vec3 p1 = {0.5f * GRID_POINTS[indices[i]] + offset[0], -GRID_POINTS[indices[i + 1]] + offset[1], offset[2]};
      vec3 p2 = {0.5f * GRID_POINTS[indices[i + 2]] + offset[0], -GRID_POINTS[indices[i + 3]] + offset[1], offset[2]};
      vec3 normal = {0.0f, 0.0f, 1.0f};
      strokes.push_back({p1, p2, normal});
    }
    return strokes;
  }

  static auto generate_vertices(const std::string &text, float thickness) -> std::vector<vertex> {
    std::vector<vertex> vertices;
    uint32_t column = 0;
    uint32_t row = 0;

    for (char c : text) {
      if (c == '\n') {
        column = 0;
        row++;
        continue;
      }

      vec3 char_offset = {0.5f * static_cast<float>(column), -static_cast<float>(row), 0.0f};

      auto strokes = get_character_strokes(c, char_offset);
      for (const auto &stroke : strokes) {
        auto stroke_vertices = generate_stroke(stroke.start, stroke.end, thickness);
        vertices.insert(vertices.end(), stroke_vertices.begin(), stroke_vertices.end());
      }

      column++;
    }

    return vertices;
  }

  static auto generate_indices(const std::string &text) -> std::vector<uint32_t> {
    // Since we're generating triangles directly in generate_vertices,
    // we don't need separate indices
    return std::vector<uint32_t>();
  }

  std::string m_text;
  uint32_t m_width = 0;
  uint32_t m_lines = 1;
};

// Define the static grid points
const float char_mesh::GRID_POINTS[17] = {0.0f,    0.0625f, 0.125f,  0.1875f, 0.25f,   0.3125f, 0.375f,  0.4375f, 0.5f,
                                          0.5625f, 0.625f,  0.6875f, 0.75f,   0.8125f, 0.875f,  0.9375f, 1.0f};

// Initialize the ASCII font map
const std::unordered_map<char, std::vector<uint32_t>> char_mesh::ASCII_FONT = {
    {'!', std::vector<unsigned int>{8, 2, 8, 11, 8, 13, 8, 14}},
    {'"', std::vector<unsigned int>{5, 2, 5, 4, 11, 2, 11, 4}},
    {'#', std::vector<unsigned int>{7, 2, 3, 14, 13, 2, 9, 14, 2, 5, 14, 5, 2, 11, 14, 11}},
    {'$', std::vector<unsigned int>{2, 12, 14, 12, 14, 12, 14, 8, 14, 8, 2, 8, 2, 8, 2, 4, 2, 4, 14, 4, 8, 2, 8, 14}},
    {'%', std::vector<unsigned int>{2,  2, 2,  6,  2,  6,  6,  6,  6,  6,  6,  2,  6,  2,  2,  2,  2,  14,
                                    14, 2, 10, 10, 10, 14, 10, 14, 14, 14, 14, 14, 14, 10, 14, 10, 10, 10}},
    {'&', std::vector<unsigned int>{12, 14, 6,  6, 6, 6,  6, 4,  6, 4,  7, 2,  7, 2,  9, 2,  9,  2,  10, 4,  10, 4,
                                    10, 6,  10, 6, 4, 10, 4, 10, 4, 13, 4, 13, 6, 14, 6, 14, 10, 14, 10, 14, 12, 11}},
    {'\'', std::vector<unsigned int>{8, 2, 8, 4}},
    {'(', std::vector<unsigned int>{8, 2, 6, 6, 6, 6, 6, 10, 6, 10, 8, 14}},
    {')', std::vector<unsigned int>{8, 2, 10, 6, 10, 6, 10, 10, 10, 10, 8, 14}},
    {'*', std::vector<unsigned int>{8, 5, 8, 11, 6, 6, 10, 10, 6, 10, 10, 6}},
    {'+', std::vector<unsigned int>{8, 5, 8, 11, 5, 8, 11, 8}},
    {',', std::vector<unsigned int>{7, 14, 9, 14, 9, 14, 9, 12}},
    {'-', std::vector<unsigned int>{5, 8, 11, 8}},
    {'.', std::vector<unsigned int>{8, 13, 8, 14}},
    {'/', std::vector<unsigned int>{11, 2, 5, 14}},
    {'0', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 2, 14, 14, 2}},
    {'1', std::vector<unsigned int>{6, 4, 8, 2, 8, 2, 8, 14, 6, 14, 10, 14}},
    {'2', std::vector<unsigned int>{2, 4, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8, 2, 8, 2, 14, 2, 14, 14, 14}},
    {'3', std::vector<unsigned int>{2, 2, 14, 2, 2, 8, 14, 8, 2, 14, 14, 14, 14, 2, 14, 14}},
    {'4', std::vector<unsigned int>{2, 2, 2, 8, 14, 2, 14, 14, 2, 8, 14, 8}},
    {'5', std::vector<unsigned int>{14, 2, 2, 2, 2, 2, 2, 8, 2, 8, 14, 8, 14, 8, 14, 14, 14, 14, 2, 14, 2, 14, 2, 12}},
    {'6', std::vector<unsigned int>{14, 4, 14, 2, 14, 2, 2, 2, 2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 8, 2, 8}},
    {'7', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 2, 14}},
    {'8', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 2, 8, 14, 8}},
    {'9', std::vector<unsigned int>{14, 8, 2, 8, 2, 8, 2, 2, 2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 12}},
    {':', std::vector<unsigned int>{8, 3, 8, 5, 8, 13, 8, 11}},
    {';', std::vector<unsigned int>{8, 3, 8, 5, 7, 14, 9, 14, 9, 14, 9, 12}},
    {'<', std::vector<unsigned int>{14, 2, 2, 8, 2, 8, 14, 14}},
    {'=', std::vector<unsigned int>{2, 5, 14, 5, 2, 11, 14, 11}},
    {'>', std::vector<unsigned int>{2, 2, 14, 8, 14, 8, 2, 14}},
    {'?', std::vector<unsigned int>{2, 4, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 8, 8, 8, 8, 8, 11, 8, 13, 8, 14}},
    {'@', std::vector<unsigned int>{10, 10, 6,  10, 6,  10, 6,  6,  6,  6,  10, 6,  10, 6,  10, 12, 10, 12,
                                    12, 12, 12, 12, 14, 10, 14, 10, 14, 6,  14, 6,  10, 2,  10, 2,  6,  2,
                                    6,  2,  2,  6,  2,  6,  2,  10, 2,  10, 6,  14, 6,  14, 10, 14}},
    {'A', std::vector<unsigned int>{2, 14, 8, 2, 8, 2, 14, 14, 4, 10, 12, 10}},
    {'B', std::vector<unsigned int>{2, 2, 2, 14, 2, 2, 11, 2, 11, 2, 11, 7, 2, 7, 14, 7, 2, 14, 14, 14, 14, 14, 14, 7}},
    {'C', std::vector<unsigned int>{14, 4, 14, 2, 14, 2, 2, 2, 2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 12}},
    {'D', std::vector<unsigned int>{2, 2, 8, 2, 8, 2, 14, 5, 14, 5, 14, 11, 14, 11, 8, 14, 8, 14, 2, 14, 2, 14, 2, 2}},
    {'E', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 2, 8, 8, 8, 2, 14, 14, 14}},
    {'F', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 2, 8, 8, 8}},
    {'G', std::vector<unsigned int>{12, 4, 12, 2, 12, 2, 2, 2, 2, 2, 2, 14, 2, 14, 12, 14, 12, 14, 12, 8, 14, 8, 8, 8}},
    {'H', std::vector<unsigned int>{2, 2, 2, 14, 14, 2, 14, 14, 2, 8, 14, 8}},
    {'I', std::vector<unsigned int>{2, 2, 14, 2, 2, 14, 14, 14, 8, 2, 8, 14}},
    {'J', std::vector<unsigned int>{2, 2, 14, 2, 11, 2, 11, 11, 11, 11, 8, 14, 8, 14, 5, 14, 5, 14, 2, 11}},
    {'K', std::vector<unsigned int>{2, 2, 2, 14, 2, 8, 14, 2, 2, 8, 14, 14}},
    {'L', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14}},
    {'M', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 8, 11, 8, 11, 14, 2, 14, 2, 14, 14}},
    {'N', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 14, 14, 14, 14, 2}},
    {'O', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2}},
    {'P', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8}},
    {'Q', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 11, 11, 16, 16}},
    {'R', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8, 8, 8, 14, 14}},
    {'S', std::vector<unsigned int>{14, 5, 14, 2, 14, 2,  2,  2,  2, 2,  2, 8,  2, 8,
                                    14, 8, 14, 8, 14, 14, 14, 14, 2, 14, 2, 14, 2, 11}},
    {'T', std::vector<unsigned int>{2, 2, 14, 2, 8, 2, 8, 14}},
    {'U', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 2}},
    {'V', std::vector<unsigned int>{2, 2, 8, 14, 8, 14, 14, 2}},
    {'W', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 8, 5, 8, 5, 14, 14, 14, 14, 14, 2}},
    {'X', std::vector<unsigned int>{2, 2, 14, 14, 14, 2, 2, 14}},
    {'Y', std::vector<unsigned int>{2, 2, 8, 8, 8, 8, 14, 2, 8, 8, 8, 14}},
    {'Z', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 2, 14, 2, 14, 14, 14}},
    {'[', std::vector<unsigned int>{8, 0, 2, 0, 2, 0, 2, 16, 2, 16, 8, 16}},
    {'\\', std::vector<unsigned int>{5, 2, 11, 14}},
    {']', std::vector<unsigned int>{8, 0, 14, 0, 14, 0, 14, 16, 14, 16, 8, 16}},
    {'^', std::vector<unsigned int>{5, 5, 8, 2, 8, 2, 11, 5}},
    {'_', std::vector<unsigned int>{0, 16, 16, 16}},
    {'`', std::vector<unsigned int>{7, 2, 9, 5}},
    {'a', std::vector<unsigned int>{2, 10, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 10, 2, 10, 14, 8, 2, 8}},
    {'b', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 8, 2, 8}},
    {'c', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 8, 2, 8}},
    {'d', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 14, 14, 2, 14, 8, 2, 8}},
    {'e', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 11, 14, 8, 14, 8, 2, 8, 2, 11, 14, 11}},
    {'f', std::vector<unsigned int>{8, 14, 8, 2, 8, 2, 14, 2, 14, 2, 14, 5, 2, 8, 14, 8}},
    {'g', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 16, 14, 8, 14, 8, 2, 8, 14, 16, 2, 16}},
    {'h', std::vector<unsigned int>{2, 14, 2, 2, 2, 8, 14, 8, 14, 8, 14, 14}},
    {'i', std::vector<unsigned int>{8, 5, 8, 14, 8, 2, 8, 3}},
    {'j', std::vector<unsigned int>{8, 5, 8, 16, 8, 16, 2, 16, 8, 2, 8, 3}},
    {'k', std::vector<unsigned int>{2, 2, 2, 14, 2, 11, 14, 8, 2, 11, 14, 14}},
    {'l', std::vector<unsigned int>{8, 2, 8, 14, 8, 14, 10, 14, 8, 2, 6, 2}},
    {'m', std::vector<unsigned int>{2, 8, 2, 14, 8, 8, 8, 14, 14, 8, 14, 14, 2, 8, 14, 8}},
    {'n', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 8, 14, 8}},
    {'o', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14}},
    {'p', std::vector<unsigned int>{2, 8, 2, 16, 14, 8, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14}},
    {'q', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 16, 2, 8, 14, 8, 2, 14, 14, 14}},
    {'r', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 11, 2, 8, 14, 8}},
    {'s', std::vector<unsigned int>{2, 8, 2, 11, 14, 11, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14, 2, 11, 14, 11}},
    {'t', std::vector<unsigned int>{8, 2, 8, 14, 2, 5, 14, 5, 8, 14, 14, 14}},
    {'u', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 14, 14, 14}},
    {'v', std::vector<unsigned int>{2, 8, 8, 14, 8, 14, 14, 8}},
    {'w', std::vector<unsigned int>{2, 8, 5, 14, 5, 14, 8, 8, 8, 8, 11, 14, 11, 14, 14, 8}},
    {'x', std::vector<unsigned int>{2, 8, 14, 14, 14, 8, 2, 14}},
    {'y', std::vector<unsigned int>{2, 8, 8, 14, 6, 16, 14, 8}},
    {'z', std::vector<unsigned int>{2, 8, 14, 8, 14, 8, 2, 14, 2, 14, 14, 14}},
    {'{', std::vector<unsigned int>{10, 2, 8, 2, 8, 2, 7, 4,  7, 4,  7, 7,  7, 7,  5,  8,
                                    5,  8, 7, 9, 7, 9, 7, 12, 7, 12, 8, 14, 8, 14, 10, 14}},
    {'|', std::vector<unsigned int>{8, 0, 8, 16}},
    {'}', std::vector<unsigned int>{6,  2, 8, 2, 8, 2, 9, 4,  9, 4,  9, 7,  9, 7,  11, 8,
                                    11, 8, 9, 9, 9, 9, 9, 12, 9, 12, 8, 14, 8, 14, 6,  14}},
    {'~', std::vector<unsigned int>{2,  10, 2,  8,  2,  8,  4,  6,  4,  6, 6,  6, 6,  6,
                                    10, 10, 10, 10, 12, 10, 12, 10, 14, 8, 14, 8, 14, 6}}};

} // namespace mareweb
#endif // MAREWEB_PRIMITIVE_MESH_HPP