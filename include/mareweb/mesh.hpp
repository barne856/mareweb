#ifndef MAREWEB_MESH_HPP
#define MAREWEB_MESH_HPP

#include "mareweb/buffer.hpp"
#include "mareweb/components/camera.hpp"
#include "mareweb/material.hpp"
#include "mareweb/pipeline.hpp"
#include "mareweb/vertex_attributes.hpp"
#include "webgpu/webgpu_cpp.h"
#include <memory>
#include <vector>

namespace mareweb {

class mesh {
public:
  // Constructor for fully specified vertex data
  mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<vertex> &vertices,
       const vertex_layout &layout, const std::vector<uint32_t> &indices = {});

  mesh(const mesh &other) = delete;
  auto operator=(const mesh &other) -> mesh & = delete;
  mesh(mesh &&other) noexcept
      : m_vertex_buffer(std::move(other.m_vertex_buffer)), m_index_buffer(std::move(other.m_index_buffer)),
        m_vertex_layout(std::move(other.m_vertex_layout)), m_primitive_state(other.m_primitive_state) {}
  auto operator=(mesh &&other) noexcept -> mesh & {
    if (this != &other) {
      m_vertex_buffer = std::move(other.m_vertex_buffer);
      m_index_buffer = std::move(other.m_index_buffer);
      m_vertex_layout = std::move(other.m_vertex_layout);
      m_primitive_state = other.m_primitive_state;
    }
    return *this;
  }
  virtual ~mesh() = default;

  [[nodiscard]] auto get_vertex_buffer() const -> const vertex_buffer & { return *m_vertex_buffer; }
  [[nodiscard]] auto get_index_buffer() const -> const index_buffer * { return m_index_buffer.get(); }
  [[nodiscard]] auto get_vertex_layout() const -> const vertex_layout & { return m_vertex_layout; }
  [[nodiscard]] auto get_vertex_count() const -> uint32_t;
  [[nodiscard]] auto get_index_count() const -> uint32_t;
  [[nodiscard]] auto get_primitive_state() const -> const wgpu::PrimitiveState & { return m_primitive_state; }
  [[nodiscard]] auto get_vertex_state() const -> vertex_state {
    vertex_state state;
    state.has_normals = m_vertex_layout.has_normals();
    state.has_texcoords = m_vertex_layout.has_texcoords();
    state.has_colors = m_vertex_layout.has_colors();
    state.is_indexed = m_index_buffer != nullptr;
    return state;
  }

  void bind_material(material &material, wgpu::RenderPassEncoder &pass_encoder) const;

  auto get_vertex_buffer() -> wgpu::Buffer { return m_vertex_buffer->get_buffer(); }
  auto get_index_buffer() -> wgpu::Buffer { return m_index_buffer->get_buffer(); }

private:
  std::unique_ptr<vertex_buffer> m_vertex_buffer;
  std::unique_ptr<index_buffer> m_index_buffer;
  vertex_layout m_vertex_layout;
  wgpu::PrimitiveState m_primitive_state;

  void create_vertex_buffer(wgpu::Device &device, const std::vector<vertex> &vertices, const vertex_layout &layout);
};

// TODO, primative meshes inherit from base_mesh and implement
// entities for renderable_mesh, renderable instanced mesh, renderable composite mesh
// e.g.
// template <material_like Material, mesh_like Mesh>
// class renderable_mesh : public entity<renderable_mesh>, public transform, public Material, public Mesh
// renderable_mesh(scene* s); // construct with scene
//
// template <material_like Material, mesh_like Mesh>
// class renderable_instanced_mesh : public entity<renderable_instanced_mesh>, public transform, public Material, public
// Mesh renderable_instanced_mesh(scene* s); // construct with scene
//
// template <material_like Material, mesh_like Mesh>
// class renderable_composite_mesh : public entity<renderable_composite_mesh>, public transform, public Material, public
// Mesh renderable_composite_mesh(scene* s); // construct with scene
//

// class instanced_mesh : public base_mesh {
// public:
//   instanced_mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state, const std::vector<vertex>
//   &vertices,
//                  const vertex_layout &layout, const std::vector<uint32_t> &indices = {})
//       : base_mesh(device, primitive_state, vertices, layout, indices) {}
//
//   void render(wgpu::RenderPassEncoder &pass_encoder, material &material, const camera &camera) const override;
//
// private:
//   void create_instance_buffer(wgpu::Device &device, const std::vector<transform> &instances);
//   std::unique_ptr<storage_buffer> m_instance_buffer;
// };

} // namespace mareweb
#endif // MAREWEB_MESH_HPP