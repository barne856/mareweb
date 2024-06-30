#ifndef MAREWEB_OBJECT_HPP
#define MAREWEB_OBJECT_HPP

#include "system.hpp"
#include <memory>
#include <vector>
#include <algorithm>

namespace mareweb {

class object {
public:
    object() {};
    virtual ~object() = default;

    virtual void on_enter() {}
    virtual void on_exit() {}
    virtual void update(float dt) {}
    virtual void render(float dt) {}

    virtual bool on_key(const key_event& event) { return false; }
    virtual bool on_mouse_button(const mouse_button_event& event) { return false; }
    virtual bool on_mouse_move(const mouse_move_event& event) { return false; }
    virtual bool on_mouse_wheel(const mouse_scroll_event& event) { return false; }
    virtual bool on_resize(const window_resize_event& event) { return false; }

    template <typename T, typename... args>
    T* create_child(args&&... a) {
        auto child = std::make_unique<T>(std::forward<args>(a)...);
        T* child_ptr = child.get();
        m_children.push_back(std::move(child));
        return child_ptr;
    }

    void add_child(std::unique_ptr<object> child) {
        m_children.push_back(std::move(child));
    }

    void remove_child(object* child) {
        auto it = std::find_if(m_children.begin(), m_children.end(),
                               [child](const auto& c) { return c.get() == child; });
        if (it != m_children.end()) {
            m_children.erase(it);
        }
    }

    const std::vector<std::unique_ptr<object>>& get_children() const {
        return m_children;
    }

protected:
    std::vector<std::unique_ptr<object>> m_children;
};

} // namespace mareweb

#endif // MAREWEB_OBJECT_HPP