#include "mareweb/components/camera.hpp"
#include <cmath>
#include <squint/geometry.hpp>

namespace mareweb {

camera::camera(float fov, float aspect_ratio, float near, float far)
    : m_fov(fov), m_aspect_ratio(aspect_ratio), m_near(near), m_far(far), m_is_perspective(true) {
    update_projection_matrix();
}

void camera::set_perspective(float fov, float aspect_ratio, float near, float far) {
    m_fov = fov;
    m_aspect_ratio = aspect_ratio;
    m_near = near;
    m_far = far;
    m_is_perspective = true;
    update_projection_matrix();
}

void camera::set_orthographic(float left, float right, float bottom, float top, float near, float far) {
    m_near = near;
    m_far = far;
    m_is_perspective = false;
    m_projection_matrix = squint::ortho(left, right, bottom, top, near, far);
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

void camera::set_near(float near) {
    m_near = near;
    update_projection_matrix();
}

void camera::set_far(float far) {
    m_far = far;
    update_projection_matrix();
}

auto camera::get_fov() const -> float {
    return m_fov;
}

auto camera::get_aspect_ratio() const -> float {
    return m_aspect_ratio;
}

auto camera::get_near() const -> float {
    return m_near;
}

auto camera::get_far() const -> float {
    return m_far;
}

void camera::update_projection_matrix() {
    if (m_is_perspective) {
        float f = 1.0f / std::tan(m_fov * 0.5f * M_PI / 180.0f);
        m_projection_matrix = squint::mat4::eye();
        m_projection_matrix[0, 0] = f / m_aspect_ratio;
        m_projection_matrix[1, 1] = f;
        m_projection_matrix[2, 2] = (m_far + m_near) / (m_near - m_far);
        m_projection_matrix[2, 3] = (2.0f * m_far * m_near) / (m_near - m_far);
        m_projection_matrix[3, 2] = -1.0f;
        m_projection_matrix[3, 3] = 0.0f;
    }
}

} // namespace mareweb
