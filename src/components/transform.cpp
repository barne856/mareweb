#include "mareweb/components/transform.hpp"
#include "squint/geometry.hpp"
#include <cmath>

namespace mareweb {

transform::transform() : m_transformation_matrix(mat4::eye()), m_unit_length(units::length::meters(1)) {}

transform::transform(const mat4 &transform_matrix)
    : m_transformation_matrix(transform_matrix), m_unit_length(units::length::meters(1)) {}

auto transform::get_position() const -> vec3_t<units::length> {
  return vec3_t<units::length>({m_transformation_matrix[0, 3] * m_unit_length,
                                m_transformation_matrix[1, 3] * m_unit_length,
                                m_transformation_matrix[2, 3] * m_unit_length});
}

auto transform::get_scale() const -> vec3 {
  return vec3({std::sqrt(m_transformation_matrix[0, 0] * m_transformation_matrix[0, 0] +
                         m_transformation_matrix[1, 0] * m_transformation_matrix[1, 0] +
                         m_transformation_matrix[2, 0] * m_transformation_matrix[2, 0]),
               std::sqrt(m_transformation_matrix[0, 1] * m_transformation_matrix[0, 1] +
                         m_transformation_matrix[1, 1] * m_transformation_matrix[1, 1] +
                         m_transformation_matrix[2, 1] * m_transformation_matrix[2, 1]),
               std::sqrt(m_transformation_matrix[0, 2] * m_transformation_matrix[0, 2] +
                         m_transformation_matrix[1, 2] * m_transformation_matrix[1, 2] +
                         m_transformation_matrix[2, 2] * m_transformation_matrix[2, 2])});
}

auto transform::get_translation_matrix() const -> mat4 {
  auto translation = mat4::eye();
  translation[0, 3] = m_transformation_matrix[0, 3];
  translation[1, 3] = m_transformation_matrix[1, 3];
  translation[2, 3] = m_transformation_matrix[2, 3];
  return translation;
}

auto transform::get_rotation_matrix() const -> mat4 {
  auto rotation = m_transformation_matrix;
  auto scale = get_scale();
  rotation[0, 0] /= scale[0];
  rotation[1, 0] /= scale[0];
  rotation[2, 0] /= scale[0];
  rotation[0, 1] /= scale[1];
  rotation[1, 1] /= scale[1];
  rotation[2, 1] /= scale[1];
  rotation[0, 2] /= scale[2];
  rotation[1, 2] /= scale[2];
  rotation[2, 2] /= scale[2];
  rotation[0, 3] = 0;
  rotation[1, 3] = 0;
  rotation[2, 3] = 0;
  return rotation;
}

auto transform::get_scale_matrix() const -> mat4 {
  auto scale_matrix = mat4::eye();
  auto scale = get_scale();
  scale_matrix[0, 0] = scale[0];
  scale_matrix[1, 1] = scale[1];
  scale_matrix[2, 2] = scale[2];
  return scale_matrix;
}

auto transform::get_transformation_matrix() const -> const mat4 & { return m_transformation_matrix; }

void transform::set_transformation_matrix(const mat4 &transformation_matrix) {
  m_transformation_matrix = transformation_matrix;
}

auto transform::get_normal_matrix() const -> mat3 {
  auto normal_matrix = mat3{m_transformation_matrix.subview<3, 3>(slice{0, 3}, slice{0, 3})};
  return normal_matrix.inv().transpose();
}

auto transform::get_view_matrix() const -> mat4 { return m_transformation_matrix.inv(); }

void transform::face_towards(const vec3_t<units::length> &point, const vec3 &up) {
  auto position = get_position();
  auto direction = (point - position) / m_unit_length;
  auto right = cross(direction, up);
  auto corrected_up = cross(right, direction);

  auto rotation_matrix = mat4::eye();
  rotation_matrix[0, 0] = right[0];
  rotation_matrix[1, 0] = right[1];
  rotation_matrix[2, 0] = right[2];
  rotation_matrix[0, 1] = corrected_up[0];
  rotation_matrix[1, 1] = corrected_up[1];
  rotation_matrix[2, 1] = corrected_up[2];
  rotation_matrix[0, 2] = -direction[0];
  rotation_matrix[1, 2] = -direction[1];
  rotation_matrix[2, 2] = -direction[2];

  m_transformation_matrix = rotation_matrix * get_translation_matrix() * get_scale_matrix();
}

void transform::translate(const vec3_t<units::length> &offset) {
  squint::translate(m_transformation_matrix, offset, m_unit_length);
}

void transform::set_position(const vec3_t<units::length> &position) {
  m_transformation_matrix[0, 3] = position[0] / m_unit_length;
  m_transformation_matrix[1, 3] = position[1] / m_unit_length;
  m_transformation_matrix[2, 3] = position[2] / m_unit_length;
}

void transform::rotate(const vec3 &axis, float angle) { squint::rotate(m_transformation_matrix, angle, axis); }

void transform::set_rotation(const vec3 &axis, float angle) {
  auto rotation_matrix = mat4::eye();
  squint::rotate(rotation_matrix, angle, axis);
  set_rotation_matrix(rotation_matrix);
}

void transform::set_rotation_matrix(const mat4 &rotation_matrix) {
  auto scale = get_scale();
  auto translation = get_translation_matrix();
  auto scale_mat = mat4::eye();
  squint::scale(scale_mat, scale);
  m_transformation_matrix = translation * rotation_matrix * scale_mat;
}

void transform::set_scale(const vec3 &scale) {
  auto rotation = get_rotation_matrix();
  auto translation = get_translation_matrix();
  auto scale_mat = mat4::eye();
  squint::scale(scale_mat, scale);
  m_transformation_matrix = translation * rotation * scale_mat;
}

auto transform::get_forward_vector() const -> vec3 {
  return vec3({-m_transformation_matrix[0, 2], -m_transformation_matrix[1, 2], -m_transformation_matrix[2, 2]});
}

auto transform::get_right_vector() const -> vec3 {
  return vec3({m_transformation_matrix[0, 0], m_transformation_matrix[1, 0], m_transformation_matrix[2, 0]});
}

auto transform::get_up_vector() const -> vec3 {
  return vec3({m_transformation_matrix[0, 1], m_transformation_matrix[1, 1], m_transformation_matrix[2, 1]});
}

void transform::set_unit_length(const units::length &unit_length) {
  auto position = get_position();
  m_unit_length = unit_length;
  set_position(position);
}

} // namespace mareweb