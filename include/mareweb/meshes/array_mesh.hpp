#ifndef MAREWEB_ARRAY_MESH_HPP
#define MAREWEB_ARRAY_MESH_HPP

#include "mareweb/mesh.hpp"
#include <vector>

namespace mareweb {

class array_mesh : public mesh {
public:
  /**
   * Constructs a mesh from raw vertex and index data with the specified layout.
   */
  array_mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<vertex> &vertices,
             const vertex_layout &layout, const std::vector<uint32_t> &indices = {})
      : mesh(device, primitive_state, vertices, layout, indices) {}

  /**
   * Constructs a mesh with default primitive state (triangles, CCW winding).
   */
  array_mesh(wgpu::Device &device, const std::vector<vertex> &vertices, const vertex_layout &layout,
             const std::vector<uint32_t> &indices = {})
      : mesh(device, get_default_primitive_state(), vertices, layout, indices) {}

  /**
   * Constructs a mesh from raw interleaved attribute data with the specified layout.
   * The data is expected to be packed according to the provided layout.
   */
  array_mesh(wgpu::Device &device, const std::vector<float> &vertex_data, const vertex_layout &layout,
             const std::vector<uint32_t> &indices = {})
      : mesh(device, get_default_primitive_state(), convert_to_vertices(vertex_data, layout), layout, indices) {}

private:
  static auto get_default_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }

  static auto convert_to_vertices(const std::vector<float> &vertex_data,
                                  const vertex_layout &layout) -> std::vector<vertex> {
    const size_t stride = layout.get_stride() / sizeof(float);
    const size_t vertex_count = vertex_data.size() / stride;
    std::vector<vertex> vertices(vertex_count);

    for (size_t i = 0; i < vertex_count; ++i) {
      vertex &vert = vertices[i];
      const float *data = vertex_data.data() + (i * stride);
      size_t offset = 0;

      for (const auto &attr : layout.get_attributes()) {
        switch (attr.location) {
        case attribute_locations::POSITION:
          std::copy(data + offset, data + offset + 3, vert.position);
          offset += 3;
          break;
        case attribute_locations::NORMAL:
          std::copy(data + offset, data + offset + 3, vert.normal);
          offset += 3;
          break;
        case attribute_locations::TEXCOORD:
          std::copy(data + offset, data + offset + 2, vert.texcoord);
          offset += 2;
          break;
        case attribute_locations::COLOR:
          std::copy(data + offset, data + offset + 4, vert.color);
          offset += 4;
          break;
        }
      }
    }

    return vertices;
  }
};

} // namespace mareweb

#endif // MAREWEB_ARRAY_MESH_HPP