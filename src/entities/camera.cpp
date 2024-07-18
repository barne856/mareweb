#include "mareweb/entities/camera.hpp"
#include <squint/geometry.hpp>

using namespace squint::units;

namespace mareweb {

camera::camera(float fov, float aspect_ratio, length near, length far)
    : m_fov(fov), m_aspect_ratio(aspect_ratio), m_near(near), m_far(far), m_is_perspective(true), m_unit_length(1) {
  update_projection_matrix();
}

void camera::set_perspective(float fov, float aspect_ratio, length near, length far) {
  m_fov = fov;
  m_aspect_ratio = aspect_ratio;
  m_near = near;
  m_far = far;
  m_is_perspective = true;
  update_projection_matrix();
}

void camera::set_orthographic(length left, length right, length bottom, length top, length near, length far) {
  m_near = near;
  m_far = far;
  m_is_perspective = false;
  m_projection_matrix = squint::ortho(left, right, bottom, top, near, far, m_unit_length);
}

auto camera::get_projection_matrix() const -> squint::mat4 { return m_projection_matrix; }

auto camera::get_view_projection_matrix() const -> squint::mat4 { return m_projection_matrix * get_view_matrix(); }

void camera::look_at(const squint::vec3_t<length> &target, const squint::vec3 &up) {
  const auto eye = get_position();
  vec3 z = squint::normalize(eye - target);
  vec3 x = squint::normalize(squint::cross(up, z));
  vec3 y = squint::cross(z, x);
  float wx = dot(x, eye) / m_unit_length;
  float wy = dot(y, eye) / m_unit_length;
  float wz = dot(z, eye) / m_unit_length;

  squint::mat4 rotation_matrix{x[0], y[0], z[0], 0.0F, x[1], y[1], z[1], 0.0F, x[2], y[2], z[2], 0, wx, wy, wz, 1.0F};

  set_rotation_matrix(rotation_matrix);
}

void camera::set_fov(float fov) {
  m_fov = fov;
  update_projection_matrix();
}

void camera::set_aspect_ratio(float aspect_ratio) {
  m_aspect_ratio = aspect_ratio;
  update_projection_matrix();
}

void camera::set_near(length near) {
  m_near = near;
  update_projection_matrix();
}

void camera::set_far(length far) {
  m_far = far;
  update_projection_matrix();
}

auto camera::get_fov() const -> float { return m_fov; }

auto camera::get_aspect_ratio() const -> float { return m_aspect_ratio; }

auto camera::get_near() const -> length { return m_near; }

auto camera::get_far() const -> length { return m_far; }

void camera::update_projection_matrix() {
  if (m_is_perspective) {
    m_projection_matrix = squint::perspective(m_fov, m_aspect_ratio, m_near, m_far, m_unit_length);
  }
}

} // namespace mareweb
