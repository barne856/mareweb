#include "mareweb/mesh.hpp"
#include <cstring>
#include <stdexcept>

namespace mareweb {

void base_mesh::create_vertex_buffer(wgpu::Device &device, const std::vector<vertex> &vertices,
                                     const vertex_layout &layout) {

  const size_t vertex_count = vertices.size();
  const size_t stride = layout.get_stride();
  std::vector<uint8_t> buffer_data(vertex_count * stride, 0);

  for (size_t i = 0; i < vertex_count; ++i) {
    uint8_t *vertex_data = buffer_data.data() + (i * stride);
    const vertex &src_vertex = vertices[i];

    for (const auto &attr : layout.get_attributes()) {
      const uint8_t *src_ptr = nullptr;
      size_t copy_size = 0;

      switch (attr.location) {
      case attribute_locations::POSITION:
        src_ptr = reinterpret_cast<const uint8_t *>(src_vertex.position);
        copy_size = 3 * sizeof(float);
        break;
      case attribute_locations::NORMAL:
        src_ptr = reinterpret_cast<const uint8_t *>(src_vertex.normal);
        copy_size = 3 * sizeof(float);
        break;
      case attribute_locations::TEXCOORD:
        src_ptr = reinterpret_cast<const uint8_t *>(src_vertex.texcoord);
        copy_size = 2 * sizeof(float);
        break;
      case attribute_locations::COLOR:
        src_ptr = reinterpret_cast<const uint8_t *>(src_vertex.color);
        copy_size = 4 * sizeof(float);
        break;
      }

      if (src_ptr && copy_size > 0) {
        std::memcpy(vertex_data + attr.offset, src_ptr, copy_size);
      }
    }
  }

  m_vertex_buffer = std::make_unique<vertex_buffer>(device, buffer_data.data(), buffer_data.size(), layout);
}

base_mesh::base_mesh(wgpu::Device &device, const wgpu::PrimitiveState &primitive_state,
                     const std::vector<vertex> &vertices, const vertex_layout &layout,
                     const std::vector<uint32_t> &indices)
    : m_primitive_state(primitive_state), m_vertex_layout(layout) {

  if (vertices.empty()) {
    throw std::runtime_error("Vertex data is empty");
  }

  // Ensure the layout is complete before creating the buffer
  if (m_vertex_layout.get_attributes().empty()) {
    throw std::runtime_error("Vertex layout has no attributes");
  }

  // Create vertex buffer with the final stride
  create_vertex_buffer(device, vertices, m_vertex_layout);

  if (!indices.empty()) {
    m_index_buffer = std::make_unique<index_buffer>(device, indices);
  }
}

auto base_mesh::get_vertex_count() const -> uint32_t {
  auto stride = m_vertex_layout.get_stride();
  if (stride == 0) {
    throw std::runtime_error("Invalid vertex layout stride");
  }
  return static_cast<uint32_t>(m_vertex_buffer->get_size() / stride);
}

auto base_mesh::get_index_count() const -> uint32_t {
  return m_index_buffer ? static_cast<uint32_t>(m_index_buffer->get_size() / sizeof(uint32_t)) : 0;
}

void base_mesh::bind_material(material &material, wgpu::RenderPassEncoder &pass_encoder) const {
  auto mesh_state = get_vertex_state();
  auto &requirements = material.get_requirements();

  // Validate compatibility
  if (!requirements.is_satisfied_by(mesh_state)) {
    std::stringstream err;
    err << "Mesh incompatible with material requirements:\n"
        << "Material needs: " << (requirements.needs_normal ? "normals " : "")
        << (requirements.needs_texcoord ? "texcoords " : "") << (requirements.needs_color ? "colors " : "")
        << "\nMesh provides: " << (mesh_state.has_normals ? "normals " : "")
        << (mesh_state.has_texcoords ? "texcoords " : "") << (mesh_state.has_colors ? "colors " : "");
    throw std::runtime_error(err.str());
  }
  material.bind(pass_encoder, get_primitive_state(), get_vertex_state());
}

void mesh::render(wgpu::RenderPassEncoder &pass_encoder, material &material, const camera &camera) const {
  bind_material(material, pass_encoder);

  // set MVP and Normal matrices
  mat4 mvp = camera.get_view_projection_matrix() * get_transformation_matrix();
  mat4x3 padded_normal_matrix;
  padded_normal_matrix.subview<3, 3>(0, 0) = camera.get_normal_matrix();
  material.update_uniform(uniform_locations::MVP_MATRIX, &mvp);
  material.update_uniform(uniform_locations::NORMAL_MATRIX, &padded_normal_matrix);

  const uint32_t vertex_count = get_vertex_count();
  pass_encoder.SetVertexBuffer(0, m_vertex_buffer->get_buffer());

  if (m_index_buffer) {
    pass_encoder.SetIndexBuffer(m_index_buffer->get_buffer(), wgpu::IndexFormat::Uint32);
    pass_encoder.DrawIndexed(get_index_count());
  } else {
    pass_encoder.Draw(vertex_count);
  }
}

} // namespace mareweb