#include "mareweb/components/transform.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"
#include "mareweb/scene.hpp"
#include <iostream>
#include <squint/quantity.hpp>

namespace mareweb {

class renderable : public entity<renderable>, public transform {
public:
  renderable() : m_scene(nullptr), m_mesh(nullptr), m_material(nullptr), transform(mat4::eye()) {};
  renderable(scene *scene, mesh *mesh = nullptr, material *material = nullptr)
      : m_scene(scene), m_mesh(mesh), m_material(material), transform(mat4::eye()) {};
  void render(const squint::duration &dt) override {
    entity<renderable>::render(dt); // execute attached render systems (usually to set uniforms)
    if (!m_scene || !m_mesh || !m_material) {
      return;
    }
    auto pass_encoder = m_scene->get_render_pass();
    m_mesh->bind_material(*m_material, pass_encoder); // bind the material
                                                      // set MVP and Normal matrices
    mat4 mvp = m_scene->get_view_projection_matrix() * get_transformation_matrix();
    mat4x3 padded_normal_matrix;
    padded_normal_matrix.subview<3, 3>(0, 0) = m_scene->get_normal_matrix();
    m_material->update_uniform(uniform_locations::MVP_MATRIX, &mvp);
    m_material->update_uniform(uniform_locations::NORMAL_MATRIX, &padded_normal_matrix);
    const uint32_t vertex_count = m_mesh->get_vertex_count();
    pass_encoder.SetVertexBuffer(0, m_mesh->get_vertex_buffer());

    if (m_mesh->get_vertex_state().is_indexed) {
      pass_encoder.SetIndexBuffer(m_mesh->get_index_buffer(), wgpu::IndexFormat::Uint32);
      pass_encoder.DrawIndexed(m_mesh->get_index_count());
    } else {
      pass_encoder.Draw(vertex_count);
    }
  }
  void set_mesh(mareweb::mesh *mesh) { this->m_mesh = mesh; }
  void set_material(mareweb::material *material) { this->m_material = material; }

private:
  scene *m_scene = nullptr;
  mareweb::mesh *m_mesh = nullptr;
  mareweb::material *m_material = nullptr;
};

// TODO implement instanced_renderable
// class instanced_renderable : public entity<instanced_renderable>, public transform

} // namespace mareweb
