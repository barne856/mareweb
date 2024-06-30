#ifndef MAREWEB_ENTITY_HPP
#define MAREWEB_ENTITY_HPP

#include "object.hpp"
#include <memory>
#include <vector>
#include <concepts>

namespace mareweb {

template<typename derived>
class entity : public object {
public:
    void update(float dt) override {
        if (!m_disabled) {
            for (auto& system : m_physics_systems) {
                system->update(dt, static_cast<derived&>(*this));
            }
            object::update(dt);
        }
    }

    void render(float dt) override {
        if (!m_disabled) {
            for (auto& system : m_render_systems) {
                system->render(dt, static_cast<derived&>(*this));
            }
            object::render(dt);
        }
    }

    bool on_key(const key_event& event) {
        if (m_disabled) return false;
        for (auto it = m_controls_systems.rbegin(); it != m_controls_systems.rend(); ++it) {
            if ((*it)->on_key(event, static_cast<derived&>(*this))) return true;
        }
        return object::on_key(event);
    }

    bool on_mouse_button(const mouse_button_event& event) {
        if (m_disabled) return false;
        for (auto it = m_controls_systems.rbegin(); it != m_controls_systems.rend(); ++it) {
            if ((*it)->on_mouse_button(event, static_cast<derived&>(*this))) return true;
        }
        return object::on_mouse_button(event);
    }

    bool on_mouse_move(const mouse_move_event& event) {
        if (m_disabled) return false;
        for (auto it = m_controls_systems.rbegin(); it != m_controls_systems.rend(); ++it) {
            if ((*it)->on_mouse_move(event, static_cast<derived&>(*this))) return true;
        }
        return object::on_mouse_move(event);
    }

    bool on_mouse_wheel(const mouse_scroll_event& event) {
        if (m_disabled) return false;
        for (auto it = m_controls_systems.rbegin(); it != m_controls_systems.rend(); ++it) {
            if ((*it)->on_mouse_wheel(event, static_cast<derived&>(*this))) return true;
        }
        return object::on_mouse_wheel(event);
    }

    bool on_resize(const window_resize_event& event) {
        if (m_disabled) return false;
        for (auto it = m_controls_systems.rbegin(); it != m_controls_systems.rend(); ++it) {
            if ((*it)->on_resize(event, static_cast<derived&>(*this))) return true;
        }
        return object::on_resize(event);
    }

    template<template<class> class system_type, typename... args>
    void attach_system(args&&... a) {
        if constexpr (std::is_base_of_v<controls_system<derived>, system_type<derived>>) {
            m_controls_systems.push_back(std::make_unique<system_type<derived>>(std::forward<args>(a)...));
        } else if constexpr (std::is_base_of_v<physics_system<derived>, system_type<derived>>) {
            m_physics_systems.push_back(std::make_unique<system_type<derived>>(std::forward<args>(a)...));
        } else if constexpr (std::is_base_of_v<render_system<derived>, system_type<derived>>) {
            m_render_systems.push_back(std::make_unique<system_type<derived>>(std::forward<args>(a)...));
        }
    }

private:
    std::vector<std::unique_ptr<controls_system<derived>>> m_controls_systems;
    std::vector<std::unique_ptr<physics_system<derived>>> m_physics_systems;
    std::vector<std::unique_ptr<render_system<derived>>> m_render_systems;
};

} // namespace mareweb

#endif // MAREWEB_ENTITY_HPP