#ifndef MAREWEB_TRANSFORM_HPP
#define MAREWEB_TRANSFORM_HPP

#include "squint/linalg.hpp"
#include <concepts>

namespace mareweb {

template <typename T>
concept transformable = requires(T t) {
    { t.get_position() } -> std::same_as<squint::tensor<squint::quantities::length_f, 3>>;
    { t.get_scale() } -> std::same_as<squint::fvec3>;
    { t.get_translation_matrix() } -> std::same_as<squint::fmat4>;
    { t.get_rotation_matrix() } -> std::same_as<squint::fmat4>;
    { t.get_scale_matrix() } -> std::same_as<squint::fmat4>;
    { t.get_transformation_matrix() } -> std::same_as<const squint::fmat4 &>;
    { t.get_normal_matrix() } -> std::same_as<squint::fmat3>;
    { t.get_view_matrix() } -> std::same_as<squint::fmat4>;
    { t.face_towards(std::declval<squint::tensor<squint::quantities::length_f, 3>>(), std::declval<squint::fvec3>()) } -> std::same_as<void>;
    { t.translate(std::declval<squint::tensor<squint::quantities::length_f, 3>>()) } -> std::same_as<void>;
    { t.set_position(std::declval<squint::tensor<squint::quantities::length_f, 3>>()) } -> std::same_as<void>;
    { t.rotate(std::declval<squint::fvec3>(), 0.f) } -> std::same_as<void>;
    { t.set_rotation(std::declval<squint::fvec3>(), 0.f) } -> std::same_as<void>;
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

    squint::tensor<squint::quantities::length_f, 3> get_position() const;
    squint::fvec3 get_scale() const;
    squint::fmat4 get_translation_matrix() const;
    squint::fmat4 get_rotation_matrix() const;
    squint::fmat4 get_scale_matrix() const;
    const squint::fmat4 &get_transformation_matrix() const;
    void set_transformation_matrix(const squint::fmat4 &transformation_matrix);
    squint::fmat3 get_normal_matrix() const;
    squint::fmat4 get_view_matrix() const;
    void face_towards(const squint::tensor<squint::quantities::length_f, 3> &point, const squint::fvec3 &up);
    void translate(const squint::tensor<squint::quantities::length_f, 3> &offset);
    void set_position(const squint::tensor<squint::quantities::length_f, 3> &position);
    void rotate(const squint::fvec3 &axis, float angle);
    void set_rotation(const squint::fvec3 &axis, float angle);
    void set_rotation_matrix(const squint::fmat4 &rotation_matrix);
    void set_scale(const squint::fvec3 &scale);
    squint::fvec3 get_forward_vector() const;
    squint::fvec3 get_right_vector() const;
    squint::fvec3 get_up_vector() const;

private:
    squint::fmat4 transformation_matrix;
};

} // namespace mareweb

#endif // MAREWEB_TRANSFORM_HPP