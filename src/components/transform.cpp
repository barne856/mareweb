#include "mareweb/components/transform.hpp"
#include "squint/fixed_tensor.hpp"
#include "squint/geometry.hpp"
#include <cmath>

namespace mareweb {

transform::transform()
    : m_translation_matrix(mat4::eye()), m_rotation_matrix(mat4::eye()), m_scale_matrix(mat4::eye()),
      m_unit_length(units::length::meters(1)) {}

transform::transform(const mat4 &transform_matrix) : m_unit_length(units::length::meters(1)) {
  // Decompose the transform matrix into translation, rotation, and scale
  m_translation_matrix = mat4::eye();
  m_translation_matrix.subview<3, 1>(0, 3) = transform_matrix.subview<3, 1>(0, 3);

  vec3 scale{
    transform_matrix.subview<3,1>(0,0).norm(),
    transform_matrix.subview<3,1>(0,1).norm(),
    transform_matrix.subview<3,1>(0,2).norm()
  };

  m_scale_matrix = mat4::eye();
  m_scale_matrix.diag_view().subview<3>(0) = scale;

  m_rotation_matrix = transform_matrix;
  m_rotation_matrix.subview<3, 1>(0, 0) /= scale[0];
  m_rotation_matrix.subview<3, 1>(0, 1) /= scale[1];
  m_rotation_matrix.subview<3, 1>(0, 2) /= scale[2];
  m_rotation_matrix.subview<3, 1>(0, 3).reshape<3>() = vec3{0.0F, 0.0F, 0.0F};
}

auto transform::get_position() const -> vec3_t<units::length> {
  return m_translation_matrix.subview<3, 1>(0, 3).reshape<3>() * m_unit_length;
}

auto transform::get_scale() const -> vec3 {
  return vec3(m_scale_matrix.diag_view().subview<3>(0));
}

auto transform::get_translation_matrix() const -> const mat4 & { return m_translation_matrix; }

auto transform::get_rotation_matrix() const -> const mat4 & { return m_rotation_matrix; }

auto transform::get_scale_matrix() const -> const mat4 & { return m_scale_matrix; }

auto transform::get_transformation_matrix() const -> mat4 {
  return m_translation_matrix * m_rotation_matrix * m_scale_matrix;
}

auto transform::get_normal_matrix() const -> mat4 {
  auto normal_matrix = m_rotation_matrix * m_scale_matrix;
  return mat4{normal_matrix.inv().transpose()};
}

auto transform::get_view_matrix() const -> mat4 { return get_transformation_matrix().inv(); }

void transform::face_towards(const vec3_t<units::length> &point, const vec3 &up) {
  const auto eye = get_position();
  vec3 z = squint::normalize(eye - point);
  vec3 x = squint::normalize(squint::cross(up, z));
  vec3 y = squint::cross(z, x);

  m_rotation_matrix.subview<3, 1>(0, 0).reshape<3>() = x;
  m_rotation_matrix.subview<3, 1>(0, 1).reshape<3>() = y;
  m_rotation_matrix.subview<3, 1>(0, 2).reshape<3>() = z;
  m_rotation_matrix.subview<3, 1>(0, 3).reshape<3>() = vec3{0.0F, 0.0F, 0.0F};
  m_rotation_matrix.subview<1, 4>(3, 0).reshape<4>() = vec4{0.0F, 0.0F, 0.0F, 1.0F};
}

void transform::translate(const vec3_t<units::length> &offset) {
  m_translation_matrix.subview<3, 1>(0, 3).reshape<3>() += offset / m_unit_length;
}

void transform::set_position(const vec3_t<units::length> &position) {
  m_translation_matrix.subview<3, 1>(0, 3).reshape<3>() = position / m_unit_length;
}

void transform::rotate(const vec3 &axis, float angle) { squint::rotate(m_rotation_matrix, angle, axis); }

void transform::set_rotation(const vec3 &axis, float angle) {
  mat4 rotation = mat4::eye();
  squint::rotate(rotation, angle, axis);
  m_rotation_matrix = rotation;
}

void transform::set_rotation_matrix(const mat4 &rotation_matrix) { m_rotation_matrix = rotation_matrix; }

void transform::set_scale(const vec3 &scale) {
  m_scale_matrix.diag_view().subview<3>(0) = scale;
}

auto transform::get_forward_vector() const -> vec3 {
  return -vec3(m_rotation_matrix.subview<3, 1>(0, 2).reshape<3>());
}

auto transform::get_right_vector() const -> vec3 {
  return vec3(m_rotation_matrix.subview<3, 1>(0, 0).reshape<3>());
}

auto transform::get_up_vector() const -> vec3 {
  return vec3(m_rotation_matrix.subview<3, 1>(0, 1).reshape<3>());
}

void transform::set_unit_length(const units::length &unit_length) {
  auto position = get_position();
  m_unit_length = unit_length;
  set_position(position);
}

} // namespace mareweb