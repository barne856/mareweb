#include "mareweb/components/camera.hpp"
#include <cmath>
#include <squint/geometry.hpp>

namespace mareweb {

camera::camera(float fov, float aspect_ratio, units::length_t<float> near, units::length_t<float> far)
    : m_fov(fov), m_aspect_ratio(aspect_ratio), m_near(near), m_far(far), m_is_perspective(true) {
    update_projection_matrix();
}

void camera::set_perspective(float fov, float aspect_ratio, units::length_t<float> near, units::length_t<float> far) {
    m_fov = fov;
    m_aspect_ratio = aspect_ratio;
    m_near = near;
    m_far = far;
    m_is_perspective = true;
    update_projection_matrix();
}

void camera::set_orthographic(units::length_t<float> left, units::length_t<float> right, 
                              units::length_t<float> bottom, units::length_t<float> top, 
                              units::length_t<float> near, units::length_t<float> far) {
    m_near = near;
    m_far = far;
    m_is_perspective = false;
    m_projection_matrix = squint::ortho(left, right, bottom, top, near, far, m_unit_length);
}

auto camera::get_projection_matrix() const -> squint::mat4 {
    return m_projection_matrix;
}

auto camera::get_view_projection_matrix() const -> squint::mat4 {
    return m_projection_matrix * get_view_matrix();
}

void camera::set_fov(float fov) {
    m_fov = fov;
    update_projection_matrix();
}

void camera::set_aspect_ratio(float aspect_ratio) {
    m_aspect_ratio = aspect_ratio;
    update_projection_matrix();
}

void camera::set_near(units::length_t<float> near) {
    m_near = near;
    update_projection_matrix();
}

void camera::set_far(units::length_t<float> far) {
    m_far = far;
    update_projection_matrix();
}

auto camera::get_fov() const -> float {
    return m_fov;
}

auto camera::get_aspect_ratio() const -> float {
    return m_aspect_ratio;
}

auto camera::get_near() const -> units::length_t<float> {
    return m_near;
}

auto camera::get_far() const -> units::length_t<float> {
    return m_far;
}

void camera::update_projection_matrix() {
    if (m_is_perspective) {
        m_projection_matrix = squint::perspective(m_fov, m_aspect_ratio, m_near, m_far, m_unit_length);
    }
}

} // namespace mareweb
