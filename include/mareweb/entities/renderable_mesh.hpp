#include "mareweb/components/transform.hpp"
#include "mareweb/entity.hpp"
#include "mareweb/material.hpp"
#include "mareweb/mesh.hpp"
#include "mareweb/scene.hpp"
#include <squint/quantity.hpp>

namespace mareweb {

class renderable_mesh : public entity<renderable_mesh>, public transform {
public:
  renderable_mesh(scene *scene) : m_scene(scene) {};
  void render(const squint::duration &dt) override {
    entity<renderable_mesh>::render(dt);  // execute attached render systems (set uniforms)
    m_scene->draw_mesh(*mesh, *material); // draw the mesh
  }
  void set_mesh(std::unique_ptr<mareweb::mesh> mesh) { this->mesh = std::move(mesh); }
  void set_material(std::unique_ptr<mareweb::material> material) { this->material = std::move(material); }

protected:
  scene *m_scene;
  std::unique_ptr<mareweb::mesh> mesh;
  std::unique_ptr<mareweb::material> material;
};

} // namespace mareweb
