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
  m_translation_matrix[0, 3] = transform_matrix[0, 3];
  m_translation_matrix[1, 3] = transform_matrix[1, 3];
  m_translation_matrix[2, 3] = transform_matrix[2, 3];

  vec3 scale{
      std::sqrt(transform_matrix[0, 0] * transform_matrix[0, 0] + transform_matrix[1, 0] * transform_matrix[1, 0] +
                transform_matrix[2, 0] * transform_matrix[2, 0]),
      std::sqrt(transform_matrix[0, 1] * transform_matrix[0, 1] + transform_matrix[1, 1] * transform_matrix[1, 1] +
                transform_matrix[2, 1] * transform_matrix[2, 1]),
      std::sqrt(transform_matrix[0, 2] * transform_matrix[0, 2] + transform_matrix[1, 2] * transform_matrix[1, 2] +
                transform_matrix[2, 2] * transform_matrix[2, 2])};

  m_scale_matrix = mat4::eye();
  m_scale_matrix[0, 0] = scale[0];
  m_scale_matrix[1, 1] = scale[1];
  m_scale_matrix[2, 2] = scale[2];

  m_rotation_matrix = transform_matrix;
  m_rotation_matrix[0, 0] /= scale[0];
  m_rotation_matrix[1, 0] /= scale[0];
  m_rotation_matrix[2, 0] /= scale[0];
  m_rotation_matrix[0, 1] /= scale[1];
  m_rotation_matrix[1, 1] /= scale[1];
  m_rotation_matrix[2, 1] /= scale[1];
  m_rotation_matrix[0, 2] /= scale[2];
  m_rotation_matrix[1, 2] /= scale[2];
  m_rotation_matrix[2, 2] /= scale[2];
  m_rotation_matrix[0, 3] = 0;
  m_rotation_matrix[1, 3] = 0;
  m_rotation_matrix[2, 3] = 0;
}

auto transform::get_position() const -> vec3_t<units::length> {
  return vec3_t<units::length>({m_translation_matrix[0, 3] * m_unit_length, m_translation_matrix[1, 3] * m_unit_length,
                                m_translation_matrix[2, 3] * m_unit_length});
}

auto transform::get_scale() const -> vec3 {
  return vec3({m_scale_matrix[0, 0], m_scale_matrix[1, 1], m_scale_matrix[2, 2]});
}

auto transform::get_translation_matrix() const -> const mat4 & { return m_translation_matrix; }

auto transform::get_rotation_matrix() const -> const mat4 & { return m_rotation_matrix; }

auto transform::get_scale_matrix() const -> const mat4 & { return m_scale_matrix; }

auto transform::get_transformation_matrix() const -> mat4 {
  return m_translation_matrix * m_rotation_matrix * m_scale_matrix;
}

auto transform::get_normal_matrix() const -> mat3 {
  auto normal_matrix = mat3{(m_rotation_matrix * m_scale_matrix).subview<3, 3>(0, 0)};
  return mat3{normal_matrix.inv().transpose()};
}

auto transform::get_view_matrix() const -> mat4 { return get_transformation_matrix().inv(); }

void transform::face_towards(const vec3_t<units::length> &point, const vec3 &up) {
  const auto eye = get_position();
  vec3 z = squint::normalize(eye - point);
  vec3 x = squint::normalize(squint::cross(up, z));
  vec3 y = squint::cross(z, x);

  m_rotation_matrix =
      mat4{x[0], x[1], x[2], 0.0F, y[0], y[1], y[2], 0.0F, z[0], z[1], z[2], 0.0F, 0.0F, 0.0F, 0.0F, 1.0F};
}

void transform::translate(const vec3_t<units::length> &offset) {
  m_translation_matrix[0, 3] += offset[0] / m_unit_length;
  m_translation_matrix[1, 3] += offset[1] / m_unit_length;
  m_translation_matrix[2, 3] += offset[2] / m_unit_length;
}

void transform::set_position(const vec3_t<units::length> &position) {
  m_translation_matrix[0, 3] = position[0] / m_unit_length;
  m_translation_matrix[1, 3] = position[1] / m_unit_length;
  m_translation_matrix[2, 3] = position[2] / m_unit_length;
}

void transform::rotate(const vec3 &axis, float angle) { squint::rotate(m_rotation_matrix, angle, axis); }

void transform::set_rotation(const vec3 &axis, float angle) {
  mat4 rotation = mat4::eye();
  squint::rotate(rotation, angle, axis);
  m_rotation_matrix = rotation;
}

void transform::set_rotation_matrix(const mat4 &rotation_matrix) { m_rotation_matrix = rotation_matrix; }

void transform::set_scale(const vec3 &scale) {
  m_scale_matrix[0, 0] = scale[0];
  m_scale_matrix[1, 1] = scale[1];
  m_scale_matrix[2, 2] = scale[2];
}

auto transform::get_forward_vector() const -> vec3 {
  return vec3({-m_rotation_matrix[0, 2], -m_rotation_matrix[1, 2], -m_rotation_matrix[2, 2]});
}

auto transform::get_right_vector() const -> vec3 {
  return vec3({m_rotation_matrix[0, 0], m_rotation_matrix[1, 0], m_rotation_matrix[2, 0]});
}

auto transform::get_up_vector() const -> vec3 {
  return vec3({m_rotation_matrix[0, 1], m_rotation_matrix[1, 1], m_rotation_matrix[2, 1]});
}

void transform::set_unit_length(const units::length &unit_length) {
  auto position = get_position();
  m_unit_length = unit_length;
  set_position(position);
}

} // namespace mareweb