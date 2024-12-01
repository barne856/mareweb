#ifndef MAREWEB_SPHERE_MESH_HPP
#define MAREWEB_SPHERE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity/constants.hpp"
#include <cstdint>

namespace mareweb {

using namespace squint;

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

} // namespace mareweb
#endif // MAREWEB_SPHERE_MESH_HPP