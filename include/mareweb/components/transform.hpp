#ifndef MAREWEB_TRANSFORM_HPP
#define MAREWEB_TRANSFORM_HPP

#include "squint/quantity.hpp"
#include "squint/tensor.hpp"
#include <concepts>

namespace mareweb {
using namespace squint;

template <typename T>
concept transformable = requires(T t) {
  { t.get_position() } -> std::same_as<vec3_t<length>>;
  { t.get_scale() } -> std::same_as<vec3>;
  { t.get_translation_matrix() } -> std::same_as<const mat4 &>;
  { t.get_rotation_matrix() } -> std::same_as<const mat4 &>;
  { t.get_scale_matrix() } -> std::same_as<const mat4 &>;
  { t.get_transformation_matrix() } -> std::same_as<mat4>;
  { t.get_normal_matrix() } -> std::same_as<mat4>;
  { t.get_view_matrix() } -> std::same_as<mat4>;
  { t.face_towards(std::declval<vec3_t<length>>(), std::declval<vec3>()) } -> std::same_as<void>;
  { t.translate(std::declval<vec3_t<length>>()) } -> std::same_as<void>;
  { t.set_position(std::declval<vec3_t<length>>()) } -> std::same_as<void>;
  { t.rotate(std::declval<vec3>(), 0.F) } -> std::same_as<void>;
  { t.set_rotation(std::declval<vec3>(), 0.F) } -> std::same_as<void>;
  { t.set_rotation_matrix(std::declval<mat4>()) } -> std::same_as<void>;
  { t.set_scale(std::declval<vec3>()) } -> std::same_as<void>;
  { t.get_forward_vector() } -> std::same_as<vec3>;
  { t.get_right_vector() } -> std::same_as<vec3>;
  { t.get_up_vector() } -> std::same_as<vec3>;
};

class transform {
public:
  transform();
  explicit transform(const mat4 &transform_matrix);

  [[nodiscard]] auto get_position() const -> vec3_t<length>;
  [[nodiscard]] auto get_scale() const -> vec3;
  [[nodiscard]] auto get_translation_matrix() const -> const mat4 &;
  [[nodiscard]] auto get_rotation_matrix() const -> const mat4 &;
  [[nodiscard]] auto get_scale_matrix() const -> const mat4 &;
  [[nodiscard]] auto get_transformation_matrix() const -> mat4;
  [[nodiscard]] auto get_normal_matrix() const -> mat4;
  [[nodiscard]] auto get_view_matrix() const -> mat4;
  void face_towards(const vec3_t<length> &point, const vec3 &up);
  void translate(const vec3_t<length> &offset);
  void set_position(const vec3_t<length> &position);
  void rotate(const vec3 &axis, float angle);
  void set_rotation(const vec3 &axis, float angle);
  void set_rotation_matrix(const mat4 &rotation_matrix);
  void set_scale(const vec3 &scale);
  [[nodiscard]] auto get_forward_vector() const -> vec3;
  [[nodiscard]] auto get_right_vector() const -> vec3;
  [[nodiscard]] auto get_up_vector() const -> vec3;
  void set_unit_length(const length &unit_length);

private:
  mat4 m_translation_matrix;
  mat4 m_rotation_matrix;
  mat4 m_scale_matrix;
  length m_unit_length;

  void update_translation_matrix();
  void update_rotation_matrix();
  void update_scale_matrix();
};

} // namespace mareweb

#endif // MAREWEB_TRANSFORM_HPP