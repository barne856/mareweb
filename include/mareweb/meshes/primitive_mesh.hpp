#ifndef MAREWEB_PRIMITIVE_MESH_HPP
#define MAREWEB_PRIMITIVE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity.hpp"
#include "squint/quantity/constants.hpp"
#include "squint/tensor.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

class triangle_mesh : public mesh {
public:
  triangle_mesh(wgpu::Device &device, const vec3_t<length> &v1, const vec3_t<length> &v2, const vec3_t<length> &v3)
      : mesh(device, get_primitive_state(),
             {v1[0].value(), v1[1].value(), v1[2].value(), v2[0].value(), v2[1].value(), v2[2].value(), v3[0].value(),
              v3[1].value(), v3[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class square_mesh : public mesh {
public:
  square_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(),
             {-size.value() / 2, -size.value() / 2, 0.0f, size.value() / 2, -size.value() / 2, 0.0f, size.value() / 2,
              size.value() / 2, 0.0f, -size.value() / 2, size.value() / 2, 0.0f},
             {0, 1, 2, 2, 3, 0}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class cube_mesh : public mesh {
public:
  cube_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(),
             {// Front face
              -size.value() / 2, -size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2,
              size.value() / 2, size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2,
              size.value() / 2, size.value() / 2,
              // Back face
              -size.value() / 2, -size.value() / 2, -size.value() / 2, size.value() / 2, -size.value() / 2,
              -size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2, -size.value() / 2,
              size.value() / 2, -size.value() / 2},
             {
                 0, 1, 2, 2, 3, 0, // Front face
                 1, 5, 6, 6, 2, 1, // Right face
                 5, 4, 7, 7, 6, 5, // Back face
                 4, 0, 3, 3, 7, 4, // Left face
                 3, 2, 6, 6, 7, 3, // Top face
                 4, 5, 1, 1, 0, 4  // Bottom face
             }) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }
};

class line_mesh : public mesh {
public:
  line_mesh(wgpu::Device &device, const vec3_t<length> &start, const vec3_t<length> &end)
      : mesh(device, get_primitive_state(),
             {start[0].value(), start[1].value(), start[2].value(), end[0].value(), end[1].value(), end[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::LineList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class point_mesh : public mesh {
public:
  point_mesh(wgpu::Device &device, const vec3_t<length> &point)
      : mesh(device, get_primitive_state(), {point[0].value(), point[1].value(), point[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::PointList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class circle_mesh : public mesh {
public:
  circle_mesh(wgpu::Device &device, length radius, std::size_t n_sides)
      : mesh(device, get_primitive_state(), generate_vertices(radius, n_sides)) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleStrip;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }

  static auto generate_vertices(length radius, std::size_t n_sides) -> std::vector<float> {
    std::vector<float> vertices;
    vertices.reserve((2 * n_sides + 2) * 3); // (sides + 1) pairs of vertices * 3 coordinates per vertex
    constexpr auto two_pi = 2.0F * math_constants<float>::pi;

    // Generate alternating center and edge vertices
    for (int i = 0; i <= n_sides; i++) {
      angle angle = two_pi * static_cast<float>(i) / static_cast<float>(n_sides);

      // Center vertex
      vertices.push_back(0.0F);
      vertices.push_back(0.0F);
      vertices.push_back(0.0F);

      // Edge vertex
      vertices.push_back((radius * std::cos(angle)).value());
      vertices.push_back((radius * std::sin(angle)).value());
      vertices.push_back(0.0F);
    }

    return vertices;
  }
};

class sphere_mesh : public mesh {
public:
  // Icosahedron-based sphere constructor
  sphere_mesh(wgpu::Device &device, length radius, unsigned int recursion_level)
      : mesh(device, get_primitive_state(), generate_icosphere_vertices(radius, recursion_level)) {}

  // Latitude-longitude based sphere constructor
  sphere_mesh(wgpu::Device &device, length radius, std::size_t n_lats, std::size_t n_lngs)
      : mesh(device, get_primitive_state(), generate_latlong_vertices(radius, n_lats, n_lngs),
             generate_latlong_indices(n_lats, n_lngs)) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  // Generates vertices for icosahedron-based sphere
  static auto generate_icosphere_vertices(length radius, unsigned int recursion_level) -> std::vector<float> {
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
    std::vector<vec3_t<length>> vertices;
    vertices.reserve(count);

    // Subdivide each face of the icosahedron
    for (int i = 0; i < 20; i++) {
      vec3_t<length> v1{length{verts[3 * indes[3 * i]]}, length{verts[3 * indes[3 * i] + 1]},
                        length{verts[3 * indes[3 * i] + 2]}};
      vec3_t<length> v2{length{verts[3 * indes[3 * i + 1]]}, length{verts[3 * indes[3 * i + 1] + 1]},
                        length{verts[3 * indes[3 * i + 1] + 2]}};
      vec3_t<length> v3{length{verts[3 * indes[3 * i + 2]]}, length{verts[3 * indes[3 * i + 2] + 1]},
                        length{verts[3 * indes[3 * i + 2] + 2]}};
      subdivide(v1, v2, v3, recursion_level, vertices);
    }

    // Convert to flat array with positions and normals
    std::vector<float> data;
    data.reserve(vertices.size() * 3); // 3 for position, 3 for normal

    for (const auto &vertex : vertices) {
      vec3_t<length> scaled_pos = radius * normalize(vertex);
      vec3 normal = normalize(vec3{vertex[0].value(), vertex[1].value(), vertex[2].value()});

      // Position
      data.push_back(scaled_pos[0].value());
      data.push_back(scaled_pos[1].value());
      data.push_back(scaled_pos[2].value());
      // Normal
      // data.push_back(normal[0]);
      // data.push_back(normal[1]);
      // data.push_back(normal[2]);
    }

    return data;
  }

  // Generates vertices for latitude-longitude based sphere
  static auto generate_latlong_vertices(length radius, std::size_t n_lats, std::size_t n_lngs) -> std::vector<float> {
    n_lngs = std::max<std::size_t>(3, n_lngs);
    n_lats = std::max<std::size_t>(2, n_lats);

    std::vector<float> data;
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

        // Position
        length x = xy * std::cos(lng);
        length y = xy * std::sin(lng);
        data.push_back(x.value());
        data.push_back(y.value());
        data.push_back(z.value());

        // Normal
        // data.push_back((x / radius).value());
        // data.push_back((y / radius).value());
        // data.push_back((z / radius).value());

        // Texture coordinates
        // data.push_back(1.0F - static_cast<float>(j) / n_lngs);
        // data.push_back(1.0F - static_cast<float>(i) / n_lats);
      }
    }

    return data;
  }

  // Generates indices for latitude-longitude based sphere
  static auto generate_latlong_indices(std::size_t n_lats, std::size_t n_lngs) -> std::vector<uint32_t> {
    std::vector<uint32_t> indices;
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