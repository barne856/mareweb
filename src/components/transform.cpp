#include "mareweb/components/transform.hpp"
#include "squint/linalg.hpp"

namespace mareweb {

transform::transform() : m_transformation_matrix(squint::fmat4::I()), m_unit_length(1.0F * squint::quantities::units::meter_f) {}

transform::transform(const squint::fmat4& transform_matrix) 
    : m_transformation_matrix(transform_matrix), m_unit_length(1.0F * squint::quantities::units::meter_f) {}

auto transform::get_position() const -> fvec3_length {
    auto pos = m_transformation_matrix.at<3>(0, 3);
    return fvec3_length{pos[0] * m_unit_length, pos[1] * m_unit_length, pos[2] * m_unit_length};
}

auto transform::get_scale() const -> squint::fvec3 {
    return {
        squint::norm(m_transformation_matrix.at<3>(0, 0)),
        squint::norm(m_transformation_matrix.at<3>(0, 1)),
        squint::norm(m_transformation_matrix.at<3>(0, 2))
    };
}

auto transform::get_translation_matrix() const -> squint::fmat4 {
    return squint::translate(squint::fmat4::I(), m_transformation_matrix.at<3>(0, 3));
}

auto transform::get_rotation_matrix() const -> squint::fmat4 {
    squint::fvec3 scale = get_scale();
    squint::fmat4 rotation = squint::fmat4::I();
    for (int i = 0; i < 3; ++i) {
        if (scale[i] != 0.0F) {
            rotation.at<3>(0, i) = m_transformation_matrix.at<3>(0, i) / scale[i];
        }
    }
    return rotation;
}

auto transform::get_scale_matrix() const -> squint::fmat4 {
    return squint::scale(squint::fmat4::I(), get_scale());
}

auto transform::get_transformation_matrix() const -> const squint::fmat4& {
    return m_transformation_matrix;
}

void transform::set_transformation_matrix(const squint::fmat4& transformation_matrix) {
    m_transformation_matrix = transformation_matrix;
}

auto transform::get_normal_matrix() const -> squint::fmat3 {
    return squint::inv(m_transformation_matrix.at<3, 3>(0, 0)).transpose();
}

auto transform::get_view_matrix() const -> squint::fmat4 {
    return squint::inv(m_transformation_matrix);
}

void transform::face_towards(const fvec3_length& point, const squint::fvec3& up) {
    auto view = squint::look_at(get_position().view_as<const float>(), point.view_as<const float>(), up.as_ref());
    m_transformation_matrix = squint::inv(view) * get_scale_matrix();
}

void transform::translate(const fvec3_length& offset) {
    squint::fvec3 offset_normalized{
        offset[0] / m_unit_length,
        offset[1] / m_unit_length,
        offset[2] / m_unit_length
    };
    m_transformation_matrix.at<3>(0, 3) += offset_normalized;
}

void transform::set_position(const fvec3_length& position) {
    squint::fvec3 pos_normalized{
        position[0] / m_unit_length,
        position[1] / m_unit_length,
        position[2] / m_unit_length
    };
    m_transformation_matrix.at<3>(0, 3) = pos_normalized;
}

void transform::rotate(const squint::fvec3& axis, float angle) {
    squint::fmat4 rotation = squint::rotate(squint::fmat4::I(), angle, axis) * get_rotation_matrix();
    m_transformation_matrix = get_translation_matrix() * rotation * get_scale_matrix();
}

void transform::set_rotation(const squint::fvec3& axis, float angle) {
    squint::fmat4 rotation = squint::rotate(squint::fmat4::I(), angle, axis);
    m_transformation_matrix = get_translation_matrix() * rotation * get_scale_matrix();
}

void transform::set_rotation_matrix(const squint::fmat4& rotation_matrix) {
    m_transformation_matrix = get_translation_matrix() * rotation_matrix * get_scale_matrix();
}

void transform::set_scale(const squint::fvec3& scale) {
    squint::fmat4 scale_matrix = squint::scale(squint::fmat4::I(), scale);
    m_transformation_matrix = get_translation_matrix() * get_rotation_matrix() * scale_matrix;
}

auto transform::get_forward_vector() const -> squint::fvec3 {
    auto rotation_matrix = get_rotation_matrix();
    return -rotation_matrix.at<3>(0, 2);
}

auto transform::get_right_vector() const -> squint::fvec3 {
    auto rotation_matrix = get_rotation_matrix();
    return rotation_matrix.at<3>(0, 0);
}

auto transform::get_up_vector() const -> squint::fvec3 {
    auto rotation_matrix = get_rotation_matrix();
    return rotation_matrix.at<3>(0, 1);
}

void transform::set_unit_length(const length_f& unit_length) {
    m_unit_length = unit_length;
}

} // namespace mareweb