#pragma once

#include <mareweb/components/transform.hpp>
#include <squint/quantity.hpp>
#include <squint/tensor.hpp>

using namespace squint::units;

namespace mareweb {

constexpr float DEFAULT_FOV = float(45.0F);
constexpr float DEFAULT_ASPECT_RATIO = 1.0F;
constexpr length DEFAULT_NEAR = length(0.1F);
constexpr length DEFAULT_FAR = length(100.0F);

class camera : public transform {
public:
  camera(float fov = DEFAULT_FOV, float aspect_ratio = DEFAULT_ASPECT_RATIO, length near = DEFAULT_NEAR,
         length far = DEFAULT_FAR);

  void set_perspective(float fov, float aspect_ratio, length near, length far);
  void set_orthographic(length left, length right, length bottom, length top, length near, length far);

  auto get_projection_matrix() const -> squint::mat4;
  auto get_view_projection_matrix() const -> squint::mat4;

  void set_fov(float fov);
  void set_aspect_ratio(float aspect_ratio);
  void set_near(length near);
  void set_far(length far);

  auto get_fov() const -> float;
  auto get_aspect_ratio() const -> float;
  auto get_near() const -> length;
  auto get_far() const -> length;

private:
  float m_fov;
  float m_aspect_ratio;
  length m_near;
  length m_far;
  bool m_is_perspective;
  squint::mat4 m_projection_matrix;
  length m_unit_length;

  void update_projection_matrix();
};

} // namespace mareweb
