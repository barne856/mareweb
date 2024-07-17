#pragma once

#include <mareweb/components/transform.hpp>
#include <squint/squint.hpp>
#include <units/units.hpp>

namespace mareweb {

class camera : public transform {
public:
    camera(float fov = 45.0f, float aspect_ratio = 1.0f, float near = 0.1f, float far = 100.0f);

    void set_perspective(float fov, float aspect_ratio, float near, float far);
    void set_orthographic(float left, float right, float bottom, float top, float near, float far);

    auto get_view_matrix() const -> squint::mat4 override;
    auto get_projection_matrix() const -> squint::mat4;
    auto get_view_projection_matrix() const -> squint::mat4;

    void set_fov(float fov);
    void set_aspect_ratio(float aspect_ratio);
    void set_near(float near);
    void set_far(float far);

    auto get_fov() const -> float;
    auto get_aspect_ratio() const -> float;
    auto get_near() const -> float;
    auto get_far() const -> float;

private:
    float m_fov;
    float m_aspect_ratio;
    float m_near;
    float m_far;
    bool m_is_perspective;
    squint::mat4 m_projection_matrix;

    void update_projection_matrix();
};

} // namespace mareweb
