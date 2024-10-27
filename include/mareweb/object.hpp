#ifndef MAREWEB_OBJECT_HPP
#define MAREWEB_OBJECT_HPP

#include "system.hpp"
#include <algorithm>
#include <memory>
#include <squint/quantity.hpp>
#include <vector>

namespace mareweb {
using namespace squint;

class object {
public:
  object() = default;
  virtual ~object() = default;

  // Delete copy constructor and copy assignment operator
  object(const object &) = delete;
  auto operator=(const object &) -> object & = delete;

  // Default move constructor and move assignment operator
  object(object &&) noexcept = default;
  auto operator=(object &&) noexcept -> object & = default;

  virtual void update(const squint::duration &dt) {}
  virtual void render(const squint::duration &dt) {}

  virtual auto on_key(const key_event & /*event*/) -> bool { return false; }
  virtual auto on_mouse_button(const mouse_button_event & /*event*/) -> bool { return false; }
  virtual auto on_mouse_move(const mouse_move_event & /*event*/) -> bool { return false; }
  virtual auto on_mouse_wheel(const mouse_scroll_event & /*event*/) -> bool { return false; }
  virtual auto on_resize(const window_resize_event & /*event*/) -> bool { return false; }

  template <typename T, typename... args> auto create_object(args &&...a) -> T * {
    auto obj = std::make_unique<T>(std::forward<args>(a)...);
    T *obj_ptr = obj.get();
    m_children.push_back(std::move(obj));
    return obj_ptr;
  }

  void add_object(std::unique_ptr<object> obj) { m_children.push_back(std::move(obj)); }

  void remove_object(object *obj) {
    auto it = std::find_if(m_children.begin(), m_children.end(), [obj](const auto &c) { return c.get() == obj; });
    if (it != m_children.end()) {
      m_children.erase(it);
    }
  }

  [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<object>> & { return m_children; }

  void set_disabled(bool disabled) { m_disabled = disabled; }
  [[nodiscard]] auto is_disabled() const -> bool { return m_disabled; }

private:
  std::vector<std::unique_ptr<object>> m_children;
  bool m_disabled = false;
};

} // namespace mareweb

#endif // MAREWEB_OBJECT_HPP