#include "mareweb/components/transform.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"
#include "mareweb/scene.hpp"
#include <squint/quantity.hpp>
#include <vector>

namespace mareweb {

// todo, material must support mvp matrix
class simple_mesh : public entity<simple_mesh>, public transform {
public:
  simple_mesh(scene *scene) : m_scene(scene) {};

  void render(const squint::duration &dt) override {
    entity<simple_mesh>::render(dt);  // execute attached render systems
    m_scene->draw_mesh(*mesh, *material); // draw the mesh
  }

  void set_mesh(std::unique_ptr<mareweb::mesh> mesh) { this->mesh = std::move(mesh); }

  void set_material(std::unique_ptr<mareweb::material> material) { this->material = std::move(material); }

protected:
  scene *m_scene;
  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::material> material;
};

// TODO, material must support instancing mvp matrices and parent mvp matrix
class instanced_mesh : public simple_mesh{
    public:
      instanced_mesh(scene *scene) : simple_mesh(scene) {};

      void render(const squint::duration &dt) override {
        entity<simple_mesh>::render(dt); // execute attached render systems
        m_scene->draw_mesh(*mesh, *material); // draw the mesh
      }

      void set_mesh(std::unique_ptr<mareweb::mesh> mesh) { this->mesh = std::move(mesh); }

      void set_material(std::unique_ptr<mareweb::material> material) { this->material = std::move(material); }

      void add_transform(const transform &transform) { m_transforms.push_back(transform); }

    private:
      std::vector<transform> m_transforms;
};

// TODO, this doesn't work yet, we need to support parent transforms in the draw calls
class composite_mesh : public entity<composite_mesh>, public transform {
    public:
      composite_mesh(scene *scene) : m_scene(scene) {};

      void render(const squint::duration &dt) override {
        entity<composite_mesh>::render(dt); // execute attached render systems
        for (auto &child : m_children) {
          child->render(dt);
        }
      }

      void add_mesh(std::unique_ptr<simple_mesh> mesh) { m_children.push_back(std::move(mesh)); }

    private:
      scene *m_scene;
      std::vector<std::unique_ptr<simple_mesh>> m_children;
};

} // namespace mareweb
