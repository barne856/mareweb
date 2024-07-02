#ifndef MAREWEB_TRANSFORM_HPP
#define MAREWEB_TRANSFORM_HPP

#include "squint/linalg.hpp"
#include "squint/quantity.hpp"
#include <concepts>

namespace mareweb {
using namespace squint::quantities;

// Define a length vector type
using fvec3_length = squint::tensor<length_f, 3>;

template <typename T>
concept transformable = requires(T t) {
  { t.get_position() } -> std::same_as<fvec3_length>;
  { t.get_scale() } -> std::same_as<squint::fvec3>;
  { t.get_translation_matrix() } -> std::same_as<squint::fmat4>;
  { t.get_rotation_matrix() } -> std::same_as<squint::fmat4>;
  { t.get_scale_matrix() } -> std::same_as<squint::fmat4>;
  { t.get_transformation_matrix() } -> std::same_as<const squint::fmat4 &>;
  { t.get_normal_matrix() } -> std::same_as<squint::fmat3>;
  { t.get_view_matrix() } -> std::same_as<squint::fmat4>;
  { t.face_towards(std::declval<fvec3_length>(), std::declval<squint::fvec3>()) } -> std::same_as<void>;
  { t.translate(std::declval<fvec3_length>()) } -> std::same_as<void>;
  { t.set_position(std::declval<fvec3_length>()) } -> std::same_as<void>;
  { t.rotate(std::declval<squint::fvec3>(), 0.F) } -> std::same_as<void>;
  { t.set_rotation(std::declval<squint::fvec3>(), 0.F) } -> std::same_as<void>;
  { t.set_rotation_matrix(std::declval<squint::fmat4>()) } -> std::same_as<void>;
  { t.set_scale(std::declval<squint::fvec3>()) } -> std::same_as<void>;
  { t.get_forward_vector() } -> std::same_as<squint::fvec3>;
  { t.get_right_vector() } -> std::same_as<squint::fvec3>;
  { t.get_up_vector() } -> std::same_as<squint::fvec3>;
};

class transform {
public:
  transform();
  explicit transform(const squint::fmat4 &transform_matrix);

  [[nodiscard]] auto get_position() const -> fvec3_length;
  [[nodiscard]] auto get_scale() const -> squint::fvec3;
  [[nodiscard]] auto get_translation_matrix() const -> squint::fmat4;
  [[nodiscard]] auto get_rotation_matrix() const -> squint::fmat4;
  [[nodiscard]] auto get_scale_matrix() const -> squint::fmat4;
  [[nodiscard]] auto get_transformation_matrix() const -> const squint::fmat4 &;
  void set_transformation_matrix(const squint::fmat4 &transformation_matrix);
  [[nodiscard]] auto get_normal_matrix() const -> squint::fmat3;
  [[nodiscard]] auto get_view_matrix() const -> squint::fmat4;
  void face_towards(const fvec3_length &point, const squint::fvec3 &up);
  void translate(const fvec3_length &offset);
  void set_position(const fvec3_length &position);
  void rotate(const squint::fvec3 &axis, float angle);
  void set_rotation(const squint::fvec3 &axis, float angle);
  void set_rotation_matrix(const squint::fmat4 &rotation_matrix);
  void set_scale(const squint::fvec3 &scale);
  [[nodiscard]] auto get_forward_vector() const -> squint::fvec3;
  [[nodiscard]] auto get_right_vector() const -> squint::fvec3;
  [[nodiscard]] auto get_up_vector() const -> squint::fvec3;
  void set_unit_length(const length_f &unit_length);

private:
  squint::fmat4 m_transformation_matrix;
  length_f m_unit_length;
};

} // namespace mareweb

#endif // MAREWEB_TRANSFORM_HPP