#ifndef MAREWEB_RENDERABLE_HPP
#define MAREWEB_RENDERABLE_HPP

#include "mareweb/components/transform.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"
#include "mareweb/scene.hpp"
#include <iostream>
#include <memory>
#include <squint/quantity.hpp>
#include <vector>

namespace mareweb {

// Base class for all renderables to establish common interface
class renderable_base {
public:
  virtual ~renderable_base() = default;
  virtual void render(const squint::duration &dt, const transform *parent_transform = nullptr) = 0;
};

// Basic renderable class for single mesh/material objects
class renderable : public entity<renderable>, public transform, public renderable_base {
public:
  // Default constructor
  renderable() : m_scene(nullptr), m_mesh(nullptr), m_material(nullptr), transform(mat4::eye()) {};

  // Constructor with scene, mesh, and material
  renderable(scene *scene, mesh *mesh = nullptr, material *material = nullptr)
      : m_scene(scene), m_mesh(mesh), m_material(material), transform(mat4::eye()) {};

  // Standard render override for entity interface
  void render(const squint::duration &dt) override { render(dt, nullptr); }

  // Extended render implementation that handles parent transforms
  void render(const squint::duration &dt, const transform *parent_transform) override {
    entity<renderable>::render(dt); // Execute attached render systems
    if (!m_scene || !m_mesh || !m_material) {
      return;
    }

    auto pass_encoder = m_scene->get_render_pass();
    m_mesh->bind_material(*m_material, pass_encoder); // Bind the material

    // Calculate combined transform if parent exists
    mat4 combined_transform = parent_transform
                                  ? parent_transform->get_transformation_matrix() * get_transformation_matrix()
                                  : get_transformation_matrix();

    // Set MVP and Normal matrices with combined transform
    mat4 mvp = m_scene->get_view_projection_matrix() * combined_transform;
    mat4x3 padded_normal_matrix;
    padded_normal_matrix.subview<3, 3>(0, 0) = get_normal_matrix();

    // Update uniforms with new matrices
    m_material->update_uniform(uniform_locations::MVP_MATRIX, &mvp);
    m_material->update_uniform(uniform_locations::NORMAL_MATRIX, &padded_normal_matrix);

    // Draw the mesh
    const uint32_t vertex_count = m_mesh->get_vertex_count();
    pass_encoder.SetVertexBuffer(0, m_mesh->get_vertex_buffer());

    if (m_mesh->get_vertex_state().is_indexed) {
      pass_encoder.SetIndexBuffer(m_mesh->get_index_buffer(), wgpu::IndexFormat::Uint32);
      pass_encoder.DrawIndexed(m_mesh->get_index_count());
    } else {
      pass_encoder.Draw(vertex_count);
    }
  }

  // Setters for mesh and material
  void set_mesh(mareweb::mesh *mesh) { this->m_mesh = mesh; }
  void set_material(mareweb::material *material) { this->m_material = material; }

private:
  scene *m_scene = nullptr;
  mareweb::mesh *m_mesh = nullptr;
  mareweb::material *m_material = nullptr;
};

// Instanced renderable class for efficient rendering of multiple instances
class instanced_renderable : public entity<instanced_renderable>, public transform, public renderable_base {
public:
  // Default constructor
  instanced_renderable() : m_scene(nullptr), m_mesh(nullptr), m_material(nullptr), transform(mat4::eye()) {}

  // Constructor with scene, mesh, material, and optional instances
  instanced_renderable(scene *scene, mesh *mesh = nullptr, material *material = nullptr,
                       const std::vector<transform> &instances = {})
      : m_scene(scene), m_mesh(mesh), m_material(material), transform(mat4::eye()) {
    if (!instances.empty()) {
      set_instances(instances);
    }
  }

  // Set multiple instances at once
  void set_instances(const std::vector<transform> &instances) {
    if (!m_instance_buffer) {
      auto device = m_scene->get_device();
      m_instance_buffer = std::make_unique<instance_buffer>(device, instances);
    } else {
      m_instance_buffer->update_transforms(instances);
    }
    if (m_material) {
      m_material->update_instance_buffer(m_instance_buffer->get_buffer(), m_instance_buffer->get_size());
    }
  }

  // Update a single instance transform
  void update_instance(size_t index, const transform &t) {
    if (!m_instance_buffer) {
      throw std::runtime_error("No instances have been set");
    }
    m_instance_buffer->update_transform(index, t);
  }

  // Update multiple instance transforms efficiently
  void update_instances(const std::vector<std::pair<size_t, transform>> &updates) {
    if (!m_instance_buffer) {
      throw std::runtime_error("No instances have been set");
    }
    m_instance_buffer->update_transforms(updates);
  }

  // Get the number of instances
  [[nodiscard]] auto get_instance_count() const -> uint32_t {
    return m_instance_buffer ? m_instance_buffer->get_instance_count() : 0;
  }

  // Get a specific instance transform
  [[nodiscard]] auto get_instance(size_t index) const -> const transform & {
    if (!m_instance_buffer) {
      throw std::runtime_error("No instances have been set");
    }
    return m_instance_buffer->get_transform(index);
  }

  // Get all instance transforms
  [[nodiscard]] auto get_instances() const -> const std::vector<transform> & {
    if (!m_instance_buffer) {
      throw std::runtime_error("No instances have been set");
    }
    return m_instance_buffer->get_transforms();
  }

  // Standard render override for entity interface
  void render(const squint::duration &dt) override { render(dt, nullptr); }

  // Extended render implementation that handles parent transforms
  void render(const squint::duration &dt, const transform *parent_transform) override {
    entity<instanced_renderable>::render(dt);

    if (!m_scene || !m_mesh || !m_material || !m_instance_buffer) {
      return;
    }

    auto pass_encoder = m_scene->get_render_pass();

    // Calculate combined transform if parent exists
    mat4 combined_transform = parent_transform
                                  ? parent_transform->get_transformation_matrix() * get_transformation_matrix()
                                  : get_transformation_matrix();

    // Set MVP and Normal matrices with combined transform
    mat4 mvp = m_scene->get_view_projection_matrix() * combined_transform;
    mat4x3 padded_normal_matrix;
    padded_normal_matrix.subview<3, 3>(0, 0) = get_normal_matrix();

    m_material->update_uniform(uniform_locations::MVP_MATRIX, &mvp);
    m_material->update_uniform(uniform_locations::NORMAL_MATRIX, &padded_normal_matrix);

    // Bind mesh and material (this will handle instance buffer binding)
    m_mesh->bind_material(*m_material, pass_encoder);

    // Draw with instancing
    const uint32_t vertex_count = m_mesh->get_vertex_count();
    pass_encoder.SetVertexBuffer(0, m_mesh->get_vertex_buffer());

    if (m_mesh->get_vertex_state().is_indexed) {
      pass_encoder.SetIndexBuffer(m_mesh->get_index_buffer(), wgpu::IndexFormat::Uint32);
      pass_encoder.DrawIndexed(m_mesh->get_index_count(), get_instance_count());
    } else {
      pass_encoder.Draw(vertex_count, get_instance_count());
    }
  }

  // Setters for mesh and material
  void set_mesh(mareweb::mesh *mesh) { m_mesh = mesh; }

  void set_material(mareweb::material *material) {
    m_material = material;
    if (m_material && m_instance_buffer) {
      m_material->update_instance_buffer(m_instance_buffer->get_buffer(), m_instance_buffer->get_size());
    }
  }

private:
  scene *m_scene = nullptr;
  mareweb::mesh *m_mesh = nullptr;
  mareweb::material *m_material = nullptr;
  std::unique_ptr<instance_buffer> m_instance_buffer;
};

// Composite renderable class for hierarchical scene graphs
class composite_renderable : public entity<composite_renderable>, public transform, public renderable_base {
public:
  // Default constructor
  composite_renderable() : transform(mat4::eye()) {}

  // Constructor with scene
  composite_renderable(scene *scene) : m_scene(scene), transform(mat4::eye()) {}

  // Add a child renderable to the composite
  void add_child(renderable_base *child) { m_children.push_back(child); }

  // Standard render override for entity interface
  void render(const squint::duration &dt) override { render(dt, nullptr); }

  // Extended render implementation that handles parent transforms
  void render(const squint::duration &dt, const transform *parent_transform) override {
    entity<composite_renderable>::render(dt);

    // Calculate this node's combined transform
    mat4 combined_transform = parent_transform
                                  ? parent_transform->get_transformation_matrix() * get_transformation_matrix()
                                  : get_transformation_matrix();

    // Create a temporary transform to pass down
    transform current_transform(combined_transform);

    // Render all children with the combined transform
    for (auto &child : m_children) {
      child->render(dt, &current_transform);
    }
  }

private:
  scene *m_scene = nullptr;
  std::vector<renderable_base *> m_children;
};

} // namespace mareweb

#endif // MAREWEB_RENDERABLE_HPP