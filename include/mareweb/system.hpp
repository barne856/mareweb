#ifndef MAREWEB_SYSTEM_HPP
#define MAREWEB_SYSTEM_HPP

#include <cstdint>
#include <squint/quantity.hpp>

namespace mareweb {
using namespace squint;

// event callback types
struct mouse_scroll_event {
  float x; // the amount scrolled horizontally, positive to the right and negative to the left
  float y; // the amount scrolled vertically, positive away from the user and negative toward the user
};
struct mouse_move_event {
  float x;    // window coordinates of mouse position (in pixels from left), reported if cursor is visible
  float y;    // window coordinates of mouse position (in pixels from top), reported if cursor is visible
  float xrel; // relative motion from last event in x direction (right is positive), always reported
  float yrel; // relative motion from last event in y direction (down is positive), always reported
};
struct window_resize_event {
  uint64_t width;  // horizontal pixels
  uint64_t height; // vertical pixels
};
enum class mouse_button_event {
  LEFT_MOUSE_DOWN,
  MIDDLE_MOUSE_DOWN,
  RIGHT_MOUSE_DOWN,
  LEFT_MOUSE_UP,
  MIDDLE_MOUSE_UP,
  RIGHT_MOUSE_UP,
};
enum class key_event {
  SPACE_DOWN,
  SPACE_UP,
  APOSTROPHE_DOWN,
  APOSTROPHE_UP,
  COMMA_DOWN,
  COMMA_UP,
  MINUS_DOWN,
  MINUS_UP,
  PERIOD_DOWN,
  PERIOD_UP,
  FORWARD_SLASH_DOWN,
  FORWARD_SLASH_UP,
  ZERO_DOWN,
  ZERO_UP,
  ONE_DOWN,
  ONE_UP,
  TWO_DOWN,
  TWO_UP,
  THREE_DOWN,
  THREE_UP,
  FOUR_DOWN,
  FOUR_UP,
  FIVE_DOWN,
  FIVE_UP,
  SIX_DOWN,
  SIX_UP,
  SEVEN_DOWN,
  SEVEN_UP,
  EIGHT_DOWN,
  EIGHT_UP,
  NINE_DOWN,
  NINE_UP,
  SEMICOLON_DOWN,
  SEMICOLON_UP,
  EQUAL_DOWN,
  EQUAL_UP,
  A_DOWN,
  A_UP,
  B_DOWN,
  B_UP,
  C_DOWN,
  C_UP,
  D_DOWN,
  D_UP,
  E_DOWN,
  E_UP,
  F_DOWN,
  F_UP,
  G_DOWN,
  G_UP,
  H_DOWN,
  H_UP,
  I_DOWN,
  I_UP,
  J_DOWN,
  J_UP,
  K_DOWN,
  K_UP,
  L_DOWN,
  L_UP,
  M_DOWN,
  M_UP,
  N_DOWN,
  N_UP,
  O_DOWN,
  O_UP,
  P_DOWN,
  P_UP,
  Q_DOWN,
  Q_UP,
  R_DOWN,
  R_UP,
  S_DOWN,
  S_UP,
  T_DOWN,
  T_UP,
  U_DOWN,
  U_UP,
  V_DOWN,
  V_UP,
  W_DOWN,
  W_UP,
  X_DOWN,
  X_UP,
  Y_DOWN,
  Y_UP,
  Z_DOWN,
  Z_UP,
  LEFT_BRACKET_DOWN,
  LEFT_BRACKET_UP,
  RIGHT_BRACKET_DOWN,
  RIGHT_BRACKET_UP,
  GRAVE_ACCENT_DOWN,
  GRAVE_ACCENT_UP,
  ESCAPE_DOWN,
  ESCAPE_UP,
  ENTER_DOWN,
  ENTER_UP,
  TAB_DOWN,
  TAB_UP,
  BACKSPACE_DOWN,
  BACKSPACE_UP,
  INSERT_DOWN,
  INSERT_UP,
  DELETE_DOWN,
  DELETE_UP,
  RIGHT_DOWN,
  RIGHT_UP,
  LEFT_DOWN,
  LEFT_UP,
  UP_DOWN,
  UP_UP,
  DOWN_DOWN,
  DOWN_UP,
  PAGE_UP_DOWN,
  PAGE_UP_UP,
  PAGE_DOWN_DOWN,
  PAGE_DOWN_UP,
  HOME_DOWN,
  HOME_UP,
  END_DOWN,
  END_UP,
  CAPS_LOCK_DOWN,
  CAPS_LOCK_UP,
  SCROLL_LOCK_DOWN,
  SCROLL_LOCK_UP,
  NUM_LOCK_DOWN,
  NUM_LOCK_UP,
  PRINT_SCREEN_DOWN,
  PRINT_SCREEN_UP,
  PAUSE_DOWN,
  PAUSE_UP,
  F1_DOWN,
  F1_UP,
  F2_DOWN,
  F2_UP,
  F3_DOWN,
  F3_UP,
  F4_DOWN,
  F4_UP,
  F5_DOWN,
  F5_UP,
  F6_DOWN,
  F6_UP,
  F7_DOWN,
  F7_UP,
  F8_DOWN,
  F8_UP,
  F9_DOWN,
  F9_UP,
  F10_DOWN,
  F10_UP,
  F11_DOWN,
  F11_UP,
  F12_DOWN,
  F12_UP,
  KEY_PAD_0_DOWN,
  KEY_PAD_0_UP,
  KEY_PAD_1_DOWN,
  KEY_PAD_1_UP,
  KEY_PAD_2_DOWN,
  KEY_PAD_2_UP,
  KEY_PAD_3_DOWN,
  KEY_PAD_3_UP,
  KEY_PAD_4_DOWN,
  KEY_PAD_4_UP,
  KEY_PAD_5_DOWN,
  KEY_PAD_5_UP,
  KEY_PAD_6_DOWN,
  KEY_PAD_6_UP,
  KEY_PAD_7_DOWN,
  KEY_PAD_7_UP,
  KEY_PAD_8_DOWN,
  KEY_PAD_8_UP,
  KEY_PAD_9_DOWN,
  KEY_PAD_9_UP,
  KEY_PAD_DECIMAL_DOWN,
  KEY_PAD_DECIMAL_UP,
  KEY_PAD_DIVIDE_DOWN,
  KEY_PAD_DIVIDE_UP,
  KEY_PAD_MULTIPLY_DOWN,
  KEY_PAD_MULTIPLY_UP,
  KEY_PAD_SUBTRACT_DOWN,
  KEY_PAD_SUBTRACT_UP,
  KEY_PAD_ADD_DOWN,
  KEY_PAD_ADD_UP,
  KEY_PAD_ENTER_DOWN,
  KEY_PAD_ENTER_UP,
  KEY_PAD_EQUAL_DOWN,
  KEY_PAD_EQUAL_UP,
  LEFT_SHIFT_DOWN,
  LEFT_SHIFT_UP,
  LEFT_CONTROL_DOWN,
  LEFT_CONTROL_UP,
  LEFT_ALT_DOWN,
  LEFT_ALT_UP,
  LEFT_SUPER_DOWN,
  LEFT_SUPER_UP,
  RIGHT_SHIFT_DOWN,
  RIGHT_SHIFT_UP,
  RIGHT_CONTROL_DOWN,
  RIGHT_CONTROL_UP,
  RIGHT_ALT_DOWN,
  RIGHT_ALT_UP,
  RIGHT_SUPER_DOWN,
  RIGHT_SUPER_UP,
  MENU_DOWN,
  MENU_UP,
};

template <typename T> class controls_system {
public:
  virtual ~controls_system() = default;
  controls_system() = default;
  controls_system(const controls_system &) = delete;
  auto operator=(const controls_system &) -> controls_system & = delete;
  controls_system(controls_system &&) = delete;
  auto operator=(controls_system &&) -> controls_system & = delete;

  virtual auto on_key(const key_event & /*event*/, T & /*entity*/) -> bool { return false; }
  virtual auto on_mouse_button(const mouse_button_event & /*event*/, T & /*entity*/) -> bool { return false; }
  virtual auto on_mouse_move(const mouse_move_event & /*event*/, T & /*entity*/) -> bool { return false; }
  virtual auto on_mouse_wheel(const mouse_scroll_event & /*event*/, T & /*entity*/) -> bool { return false; }
  virtual auto on_resize(const window_resize_event & /*event*/, T & /*entity*/) -> bool { return false; }
};

template <typename T> class physics_system {
public:
  virtual ~physics_system() = default;
  physics_system() = default;
  physics_system(const physics_system &) = delete;
  auto operator=(const physics_system &) -> physics_system & = delete;
  physics_system(physics_system &&) = delete;
  auto operator=(physics_system &&) -> physics_system & = delete;

  virtual void update(const squint::duration& dt, T &entity) {}
};

template <typename T> class render_system {
public:
  virtual ~render_system() = default;
  render_system() = default;
  render_system(const render_system &) = delete;
  auto operator=(const render_system &) -> render_system & = delete;
  render_system(render_system &&) = delete;
  auto operator=(render_system &&) -> render_system & = delete;

  virtual void render(const squint::duration& dt, T &entity) {}
};

} // namespace mareweb

#endif // MAREWEB_SYSTEM_HPP