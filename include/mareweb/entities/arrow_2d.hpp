#ifndef MAREWEB_ARROW_MESH_2D_HPP
#define MAREWEB_ARROW_MESH_2D_HPP

#include "mareweb/entities/renderable.hpp"
#include "mareweb/materials/flat_color_material.hpp"
#include "mareweb/meshes/square_mesh.hpp"
#include "mareweb/meshes/triangle_mesh.hpp"
#include "squint/quantity/quantity_types.hpp"
#include <memory>

namespace mareweb {

using namespace squint;

class arrow_2d : public composite_renderable {
public:
  arrow_2d(scene *scene, length body_length, length body_width, length head_length, length head_width,
           material *head_material = nullptr, material *body_material = nullptr)
      : composite_renderable(scene) {

    // Create head (triangle)
    vec3_t<length> v1{head_width * 0.5f, body_length, length(0.0f)};
    vec3_t<length> v2{length(0.0f), body_length + head_length, length(0.0f)};
    vec3_t<length> v3{-head_width * 0.5f, body_length, length(0.0f)};
    m_head_mesh = scene->create_mesh<triangle_mesh>(v1, v2, v3);

    // Create body (scaled and positioned square)
    m_body_mesh = scene->create_mesh<square_mesh>(body_width);

    // Create materials (either use provided ones or create new ones)
    if (head_material) {
      m_head_material = nullptr; // We'll use the provided material
    } else {
      m_head_material = scene->create_material<flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
      head_material = m_head_material.get();
    }

    if (body_material) {
      m_body_material = nullptr; // We'll use the provided material
    } else {
      m_body_material = scene->create_material<flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
      body_material = m_body_material.get();
    }

    // Create the renderable objects
    m_head = create_object<renderable>(scene, m_head_mesh.get(), head_material);
    m_body = create_object<renderable>(scene, m_body_mesh.get(), body_material);

    // Position and scale the body
    m_body->translate(vec3_t<length>{length(0.0f), body_length * 0.5f, length(0.0f)});
    m_body->set_scale(vec3{1.0f, body_length.value() / body_width.value(), 1.0f});

    add_child(m_head);
    add_child(m_body);
  }

  void set_head_material(material *mat) {
    if (m_head) {
      m_head->set_material(mat);
    }
  }

  void set_body_material(material *mat) {
    if (m_body) {
      m_body->set_material(mat);
    }
  }

  void set_materials(material *mat) {
    set_head_material(mat);
    set_body_material(mat);
  }

  void set_scale(const vec3 &scale) {
    m_head->set_scale(scale);
    m_body->set_scale(vec3{scale[0], scale[1], scale[2]});
  }

  void set_rotation(float angle, const vec3 &axis) {
    m_head->set_rotation(axis, angle);
    m_body->set_rotation(axis, angle);
  }

  [[nodiscard]] auto get_head() const -> renderable * { return m_head; }
  [[nodiscard]] auto get_body() const -> renderable * { return m_body; }

private:
  std::unique_ptr<mesh> m_head_mesh;
  std::unique_ptr<mesh> m_body_mesh;
  std::unique_ptr<material> m_head_material; // Owns head material if not provided
  std::unique_ptr<material> m_body_material; // Owns body material if not provided
  renderable *m_head = nullptr;
  renderable *m_body = nullptr;
};

} // namespace mareweb

#endif // MAREWEB_ARROW_MESH_2D_HPP