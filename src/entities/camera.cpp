#include "mareweb/entities/camera.hpp"
#include <squint/geometry.hpp>

using namespace squint::units;

namespace mareweb {

camera::camera(projection_type type)
    : m_type(type), m_fov(DEFAULT_FOV), m_aspect_ratio(DEFAULT_ASPECT_RATIO),
      m_perspective_near(DEFAULT_PERSPECTIVE_NEAR), m_perspective_far(DEFAULT_PERSPECTIVE_FAR),
      m_unit_length(DEFAULT_UNIT_LENGTH), m_left(DEFAULT_LEFT), m_right(DEFAULT_RIGHT), m_bottom(DEFAULT_BOTTOM),
      m_top(DEFAULT_TOP), m_orthographic_near(DEFAULT_ORTHOGRAPHIC_NEAR), m_orthographic_far(DEFAULT_ORTHOGRAPHIC_FAR),
      m_projection_matrix(squint::mat4::eye()) {
  if (m_type == projection_type::perspective) {
    set_perspective(m_fov, m_aspect_ratio, m_perspective_near, m_perspective_far);
  } else {
    set_orthographic(m_left, m_right, m_bottom, m_top, m_orthographic_near, m_orthographic_far);
  }
}

void camera::set_perspective(float fov, float aspect_ratio, length near, length far) {
  m_fov = fov;
  m_aspect_ratio = aspect_ratio;
  m_perspective_near = near;
  m_perspective_far = far;
  m_type = projection_type::perspective;
  update_projection_matrix();
}

void camera::set_orthographic(length left, length right, length bottom, length top, length near, length far) {
  m_left = left;
  m_right = right;
  m_bottom = bottom;
  m_top = top;
  m_orthographic_near = near;
  m_orthographic_far = far;
  m_type = projection_type::orthographic;
  update_projection_matrix();
}

auto camera::get_projection_matrix() const -> squint::mat4 { return m_projection_matrix; }

auto camera::get_view_projection_matrix() const -> squint::mat4 { return m_projection_matrix * get_view_matrix(); }

void camera::set_fov(float fov) {
  m_fov = fov;
  update_projection_matrix();
}

void camera::set_aspect_ratio(float aspect_ratio) {
  m_aspect_ratio = aspect_ratio;
  update_projection_matrix();
}

void camera::set_perspective_near(length near) {
  m_perspective_near = near;
  update_projection_matrix();
}

void camera::set_perspective_far(length far) {
  m_perspective_far = far;
  update_projection_matrix();
}

void camera::set_unit_length(length unit_length) {
  m_unit_length = unit_length;
  update_projection_matrix();
}

void camera::set_left(length left) {
  m_left = left;
  update_projection_matrix();
}

void camera::set_right(length right) {
  m_right = right;
  update_projection_matrix();
}

void camera::set_bottom(length bottom) {
  m_bottom = bottom;
  update_projection_matrix();
}

void camera::set_top(length top) {
  m_top = top;
  update_projection_matrix();
}

void camera::set_orthographic_near(length near) {
  m_orthographic_near = near;
  update_projection_matrix();
}

void camera::set_orthographic_far(length far) {
  m_orthographic_far = far;
  update_projection_matrix();
}

auto camera::get_fov() const -> float { return m_fov; }

auto camera::get_aspect_ratio() const -> float { return m_aspect_ratio; }

auto camera::get_perspective_near() const -> length { return m_perspective_near; }

auto camera::get_perspective_far() const -> length { return m_perspective_far; }

auto camera::get_unit_length() const -> length { return m_unit_length; }

auto camera::get_left() const -> length { return m_left; }

auto camera::get_right() const -> length { return m_right; }

auto camera::get_bottom() const -> length { return m_bottom; }

auto camera::get_top() const -> length { return m_top; }

auto camera::get_orthographic_near() const -> length { return m_orthographic_near; }

auto camera::get_orthographic_far() const -> length { return m_orthographic_far; }

void camera::update_projection_matrix() {
  switch (m_type) {
  case projection_type::perspective:
    m_projection_matrix =
        squint::perspective(m_fov, m_aspect_ratio, m_perspective_near, m_perspective_far, m_unit_length);
    break;
  case projection_type::orthographic:
    m_projection_matrix =
        squint::ortho(m_left, m_right, m_bottom, m_top, m_orthographic_near, m_orthographic_far, m_unit_length);
    break;
  }
}

} // namespace mareweb
