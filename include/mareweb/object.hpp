#ifndef MAREWEB_OBJECT_HPP
#define MAREWEB_OBJECT_HPP

#include "system.hpp"
#include <algorithm>
#include <memory>
#include <vector>

namespace mareweb {

class object {
public:
  object(){};
  virtual ~object() = default;

  virtual void update(float dt) {}
  virtual void render(float dt) {}

  virtual bool on_key(const key_event &event) { return false; }
  virtual bool on_mouse_button(const mouse_button_event &event) { return false; }
  virtual bool on_mouse_move(const mouse_move_event &event) { return false; }
  virtual bool on_mouse_wheel(const mouse_scroll_event &event) { return false; }
  virtual bool on_resize(const window_resize_event &event) { return false; }

  template <typename T, typename... args> T *create_object(args &&...a) {
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

  const std::vector<std::unique_ptr<object>> &get_children() const { return m_children; }

  void set_disabled(bool disabled) { m_disabled = disabled; }
  bool is_disabled() const { return m_disabled; }

protected:
  std::vector<std::unique_ptr<object>> m_children;
  bool m_disabled = false;
};

} // namespace mareweb

#endif // MAREWEB_OBJECT_HPP