#pragma once

#include <mareweb/components/transform.hpp>
#include <squint/quantity.hpp>
#include <squint/tensor.hpp>

using namespace squint::units;

namespace mareweb {

constexpr float DEFAULT_FOV = float(45.0F);
constexpr float DEFAULT_ASPECT_RATIO = 1.0F;
constexpr length DEFAULT_PERSPECTIVE_NEAR = length(0.1F);
constexpr length DEFAULT_PERSPECTIVE_FAR = length(100.0F);
constexpr length DEFAULT_UNIT_LENGTH = length(1.0F);
constexpr length DEFAULT_LEFT = length(-1.0F);
constexpr length DEFAULT_RIGHT = length(1.0F);
constexpr length DEFAULT_BOTTOM = length(-1.0F);
constexpr length DEFAULT_TOP = length(1.0F);
constexpr length DEFAULT_ORTHOGRAPHIC_NEAR = length(0.0F);
constexpr length DEFAULT_ORTHOGRAPHIC_FAR = length(1.0F);

enum class projection_type { perspective, orthographic };

class camera : public transform {
public:
  camera(projection_type type);

  void set_perspective(float fov, float aspect_ratio, length near, length far);
  void set_orthographic(length left, length right, length bottom, length top, length near, length far);

  [[nodiscard]] auto get_projection_matrix() const -> squint::mat4;
  [[nodiscard]] auto get_view_projection_matrix() const -> squint::mat4;

  void set_fov(float fov);
  void set_aspect_ratio(float aspect_ratio);
  void set_perspective_near(length near);
  void set_perspective_far(length far);
  void set_unit_length(length unit_length);
  void set_left(length left);
  void set_right(length right);
  void set_bottom(length bottom);
  void set_top(length top);
  void set_orthographic_near(length near);
  void set_orthographic_far(length far);

  [[nodiscard]] auto get_unit_length() const -> length;
  [[nodiscard]] auto get_left() const -> length;
  [[nodiscard]] auto get_right() const -> length;
  [[nodiscard]] auto get_bottom() const -> length;
  [[nodiscard]] auto get_top() const -> length;
  [[nodiscard]] auto get_orthographic_near() const -> length;
  [[nodiscard]] auto get_orthographic_far() const -> length;

  [[nodiscard]] auto get_fov() const -> float;
  [[nodiscard]] auto get_aspect_ratio() const -> float;
  [[nodiscard]] auto get_perspective_near() const -> length;
  [[nodiscard]] auto get_perspective_far() const -> length;

private:
  float m_fov;
  float m_aspect_ratio;
  length m_perspective_near;
  length m_perspective_far;
  length m_left;
  length m_right;
  length m_bottom;
  length m_top;
  length m_orthographic_near;
  length m_orthographic_far;
  squint::mat4 m_projection_matrix;
  length m_unit_length;
  projection_type m_type;

  void update_projection_matrix();
};

} // namespace mareweb
