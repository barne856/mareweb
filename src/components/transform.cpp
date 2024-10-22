#include "mareweb/components/transform.hpp"
#include "squint/geometry.hpp"
#include "squint/quantity.hpp"
#include "squint/tensor.hpp"
#include "squint/tensor/tensor_types.hpp"
#include <cmath>

using namespace squint;

namespace mareweb {

transform::transform()
    : m_translation_matrix(mat4::eye()), m_rotation_matrix(mat4::eye()), m_scale_matrix(mat4::eye()),
      m_unit_length(units::meters(1)) {}

transform::transform(const mat4 &transform_matrix) : m_unit_length(units::meters(1)) {
  // Decompose the transform matrix into translation, rotation, and scale
  m_translation_matrix = mat4::eye();
  m_translation_matrix.subview<3, 1>(0, 3) = transform_matrix.subview<3, 1>(0, 3);

  vec3 scale{norm(transform_matrix.subview<3, 1>(0, 0)), norm(transform_matrix.subview<3, 1>(0, 1)),
             norm(transform_matrix.subview<3, 1>(0, 2))};

  m_scale_matrix = mat4::eye();
  m_scale_matrix.diag_view().subview<3>(0) = scale;

  m_rotation_matrix = transform_matrix;
  m_rotation_matrix.subview<3, 1>(0, 0) /= scale[0];
  m_rotation_matrix.subview<3, 1>(0, 1) /= scale[1];
  m_rotation_matrix.subview<3, 1>(0, 2) /= scale[2];
  m_rotation_matrix.subview<3, 1>(0, 3) = vec3{0.0F, 0.0F, 0.0F};
}

auto transform::get_position() const -> vec3_t<squint::length> {
  return m_translation_matrix.subview<3, 1>(0, 3) * m_unit_length;
}

auto transform::get_scale() const -> vec3 { return vec3(m_scale_matrix.diag_view().subview<3>(0)); }

auto transform::get_translation_matrix() const -> const mat4 & { return m_translation_matrix; }

auto transform::get_rotation_matrix() const -> const mat4 & { return m_rotation_matrix; }

auto transform::get_scale_matrix() const -> const mat4 & { return m_scale_matrix; }

auto transform::get_transformation_matrix() const -> mat4 {
  return m_translation_matrix * m_rotation_matrix * m_scale_matrix;
}

auto transform::get_normal_matrix() const -> mat4 {
  auto normal_matrix = m_rotation_matrix * m_scale_matrix;
  return mat4{inv(normal_matrix).transpose()};
}

auto transform::get_view_matrix() const -> mat4 {
  mat4 result = mat4::eye();

  mat4 transformation_matrix = get_transformation_matrix();
  mat3 P = transformation_matrix.subview<3, 3>(0, 0);
  vec3 v = transformation_matrix.subview<3, 1>(0, 3);
  mat3 P_inv = inv(P);
  vec3 v_prime = -P_inv * v;
  result.subview<3, 1>(0, 3) = v_prime;
  result.subview<3, 3>(0, 0) = P_inv;

  return result;

  // or simply:
  // return inv(get_transformation_matrix());
}

void transform::face_towards(const vec3_t<squint::length> &point, const vec3 &up) {
  const auto eye = get_position();
  vec3 z = squint::normalize(eye - point);
  vec3 x = squint::normalize(squint::cross(up, z));
  vec3 y = squint::cross(z, x);

  m_rotation_matrix.subview<3, 1>(0, 0) = x;
  m_rotation_matrix.subview<3, 1>(0, 1) = y;
  m_rotation_matrix.subview<3, 1>(0, 2) = z;
  m_rotation_matrix.subview<3, 1>(0, 3) = vec3{0.0F, 0.0F, 0.0F};
  m_rotation_matrix.subview<1, 4>(3, 0) = (vec4{0.0F, 0.0F, 0.0F, 1.0F}).reshape<1, 4>();
}

void transform::translate(const vec3_t<squint::length> &offset) {
  m_translation_matrix.subview<3, 1>(0, 3) += offset / m_unit_length;
}

void transform::set_position(const vec3_t<squint::length> &position) {
  m_translation_matrix.subview<3, 1>(0, 3) = position / m_unit_length;
}

void transform::rotate(const vec3 &axis, float angle) { geometry::rotate(m_rotation_matrix, angle, axis); }

void transform::set_rotation(const vec3 &axis, float angle) {
  mat4 rotation = mat4::eye();
  geometry::rotate(rotation, angle, axis);
  m_rotation_matrix = rotation;
}

void transform::set_rotation_matrix(const mat4 &rotation_matrix) { m_rotation_matrix = rotation_matrix; }

void transform::set_scale(const vec3 &scale) { m_scale_matrix.diag_view().subview<3>(0) = scale; }

auto transform::get_forward_vector() const -> vec3 { return -vec3(m_rotation_matrix.subview<3, 1>(0, 2)); }

auto transform::get_right_vector() const -> vec3 { return vec3(m_rotation_matrix.subview<3, 1>(0, 0)); }

auto transform::get_up_vector() const -> vec3 { return vec3(m_rotation_matrix.subview<3, 1>(0, 1)); }

void transform::set_unit_length(const length &unit_length) {
  auto position = get_position();
  m_unit_length = unit_length;
  set_position(position);
}

} // namespace mareweb