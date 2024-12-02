#ifndef MAREWEB_TEXT_HPP
#define MAREWEB_TEXT_HPP

#include "mareweb/entities/renderable.hpp"
#include "mareweb/materials/instanced_flat_color_material.hpp"
#include "mareweb/meshes/cube_mesh.hpp"
#include "mareweb/meshes/cylinder_mesh.hpp"
#include "mareweb/meshes/line_mesh.hpp"
#include "mareweb/meshes/circle_mesh.hpp"
#include "mareweb/meshes/square_mesh.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace mareweb {
using namespace squint;

class text : public composite_renderable {
public:
    text(scene* scene, const std::string& str, float thickness = 0.0f, float extrusion = 0.0f,
         material* node_material = nullptr, material* link_material = nullptr, size_t buffer_size = 100)
        : composite_renderable(scene), m_text(str), m_thickness(thickness), m_extrusion(extrusion), m_buffer_size(buffer_size)  {
        
        if (extrusion == 0.0f) {
            if (thickness == 0.0f) {
                // 2D text with single-pixel lines
                m_link_mesh = scene->create_mesh<line_mesh>();
                if (link_material) {
                    m_link_material = nullptr;
                } else {
                    m_link_material = scene->create_material<instanced_flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    link_material = m_link_material.get();
                }
                m_link_instances = create_object<instanced_renderable>(
                    scene,
                    m_link_mesh.get(),
                    link_material,
                    buffer_size
                );
            } else {
                // 2D text with circle nodes and square links
                m_node_mesh = scene->create_mesh<circle_mesh>(length(0.5f), 16);
                m_link_mesh = scene->create_mesh<square_mesh>(length(1.F));

                // Create or use provided materials
                if (node_material) {
                    m_node_material = nullptr;
                } else {
                    m_node_material = scene->create_material<instanced_flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    node_material = m_node_material.get();
                }

                if (link_material) {
                    m_link_material = nullptr;
                } else {
                    m_link_material = scene->create_material<instanced_flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
                    link_material = m_link_material.get();
                }

                m_node_instances = create_object<instanced_renderable>(
                    scene,
                    m_node_mesh.get(),
                    node_material,
                    buffer_size * 2  // Twice the buffer size for nodes
                );
                m_link_instances = create_object<instanced_renderable>(
                    scene,
                    m_link_mesh.get(),
                    link_material,
                    buffer_size
                );
            }
        } else {
            // 3D extruded text with cylinders and cubes
            m_node_mesh = scene->create_mesh<cylinder_mesh>(length(0.5f), length(1.F), 
                                                          0, units::degrees(360.0f), 16);
            m_link_mesh = scene->create_mesh<cube_mesh>(length(1.0f));

            if (node_material) {
                m_node_material = nullptr;
            } else {
                m_node_material = scene->create_material<instanced_flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
                node_material = m_node_material.get();
            }

            if (link_material) {
              m_link_material = nullptr;
            } else {
                m_link_material = scene->create_material<instanced_flat_color_material>(vec4{1.0f, 1.0f, 1.0f, 1.0f});
                link_material = m_link_material.get();
            }

            m_node_instances = create_object<instanced_renderable>(
                scene,
                m_node_mesh.get(),
                node_material,
                buffer_size * 2  // Twice the buffer size for nodes
            );
            m_link_instances = create_object<instanced_renderable>(
                scene,
                m_link_mesh.get(),
                link_material,
                buffer_size
            );
        }
        add_child(m_node_instances);
        add_child(m_link_instances);
        update_text(str);
    }

    void set_text(const std::string& str) {
        if (str != m_text) {
            // Clear existing instances
            if (m_node_instances) m_node_instances->clear_instances();
            if (m_link_instances) m_link_instances->clear_instances();
            
            update_text(str);
            m_text = str;
        }
    }

    const std::string& get_text() const { return m_text; }

    void set_node_material(material* mat) {
        if (m_node_instances) {
            m_node_instances->set_material(mat);
        }
    }

    void set_link_material(material* mat) {
        if (m_link_instances) {
            m_link_instances->set_material(mat);
        }
    }

    void set_materials(material* mat) {
        set_node_material(mat);
        set_link_material(mat);
    }

    vec2 get_center() const {
        vec2 label_center = {m_max_width / 4.0f, m_lines / 2.0f};
        vec3 label_scale = get_scale();
        vec2 true_label_center = {
            get_position()[0].value() + 2.0f * label_scale[0] * label_center[0],
            get_position()[1].value() - 2.0f * label_scale[1] * label_center[1]
        };
        return true_label_center;
    }

    void set_center(const vec2& center) {
        vec2 label_center = {m_max_width / 4.0f, m_lines / 2.0f};
        vec3 label_scale = get_scale();
        vec2 label_top_left = {
            center[0] - label_scale[0] * label_center[0],
            center[1] + label_scale[1] * label_center[1]
        };
        set_position(vec3_t<length>{length(label_top_left[0]), length(label_top_left[1]), length(0.0f)});
    }

private:
    void update_text(const std::string& str) {
        m_lines = 0;
        m_max_width = 0;
        m_stroke_count = 0;

        // Clear existing instances
        m_node_transforms.clear();
        m_link_transforms.clear();

        unsigned int column = 0;
        unsigned int row = 0;

        for (char c : str) {
            if (c == '\n') {
                column = 0;
                row++;
                continue;
            }
            
            push_char(column, row, c);
            
            if (column + 1 > m_max_width) {
                m_max_width = column + 1;
            }
            column++;
        }
        m_lines = row + 1;

        // Update instance buffers with accumulated transforms
        if (m_node_instances && !m_node_transforms.empty()) {
            if (m_node_transforms.size() > m_buffer_size * 2) {
                throw std::runtime_error("Text requires more node instances than buffer capacity");
            }
            m_node_instances->set_instances(m_node_transforms);
        }
        
        if (m_link_instances && !m_link_transforms.empty()) {
            if (m_link_transforms.size() > m_buffer_size) {
                throw std::runtime_error("Text requires more link instances than buffer capacity");
            }
            m_link_instances->set_instances(m_link_transforms);
        }
    }

    void push_char(unsigned int column, unsigned int row, char letter) {
        auto it = ASCII_font.find(letter);
        if (it == ASCII_font.end()) {
            return;
        }

        vec3 offset{0.5f * static_cast<float>(column), -static_cast<float>(row), 0.0f};
        const auto& indices = it->second;

        for (size_t i = 0; i < indices.size() / 4; i++) {
            vec3 p1{0.5f * grid_points[indices[4 * i]], -grid_points[indices[4 * i + 1]], 0.0f};
            vec3 p2{0.5f * grid_points[indices[4 * i + 2]], -grid_points[indices[4 * i + 3]], 0.0f};
            p1 += offset;
            p2 += offset;
            vec3 p3 = 0.5f * (p1 + p2);

            // Create transforms for nodes
            if (m_node_instances) {
                transform t1, t2;
                if (m_extrusion == 0.0f && m_thickness == 0.0f) {
                  t1.set_position(vec3_t<length>{length(p1[0]), length(p1[1]), length(p1[2])});
                  t2.set_position(vec3_t<length>{length(p2[0]), length(p2[1]), length(p2[2])});
                }
                else if (m_extrusion == 0.0f) {
                  t1.set_position(vec3_t<length>{length(p1[0]), length(p1[1]), length(p1[2])});
                  t2.set_position(vec3_t<length>{length(p2[0]), length(p2[1]), length(p2[2])});
                  t1.set_scale({m_thickness, m_thickness, 1.0f});
                  t2.set_scale({m_thickness, m_thickness, 1.0f});
                } else {
                  t1.set_position(vec3_t<length>{length(p1[0]), length(p1[1]), length(-0.5f * m_extrusion)});
                  t2.set_position(vec3_t<length>{length(p2[0]), length(p2[1]), length(-0.5f * m_extrusion)});
                  t1.set_scale({m_thickness, m_thickness, m_extrusion});
                  t2.set_scale({m_thickness, m_thickness, m_extrusion});
                }
                m_node_transforms.push_back(t1);
                m_node_transforms.push_back(t2);
            }

            // Create transform for link
            transform link_transform;
            float angle = std::atan2(p2[1] - p1[1], p2[0] - p1[0]);
            float length = norm(p2 - p1);

            link_transform.set_position(
                vec3_t<squint::length>{squint::length(p3[0]), squint::length(p3[1]), squint::length(p3[2])});
            link_transform.rotate({0.0f, 0.0f, 1.0f}, angle);

            if (m_extrusion == 0.0f && m_thickness == 0.0f) {
                link_transform.set_scale({length, 1.0f, 1.0f});
            }
            else if (m_extrusion == 0.0f) {
                link_transform.set_scale({length, m_thickness, 1.0f});
            } else {
              link_transform.set_scale({length, m_thickness, m_extrusion});
            }

            m_link_transforms.push_back(link_transform);
        }
    }

private:
    std::string m_text;
    float m_thickness;
    float m_extrusion;
    size_t m_buffer_size;
    unsigned int m_lines = 0;
    unsigned int m_max_width = 0;
    unsigned int m_stroke_count = 0;
    
    std::unique_ptr<mesh> m_node_mesh;
    std::unique_ptr<mesh> m_link_mesh;
    std::unique_ptr<material> m_node_material;
    std::unique_ptr<material> m_link_material;
    instanced_renderable* m_node_instances = nullptr;
    instanced_renderable* m_link_instances = nullptr;

    std::vector<transform> m_node_transforms;
    std::vector<transform> m_link_transforms;

    static constexpr float grid_points[17] = {0.0f,   0.0625f, 0.125f, 0.1875f, 0.25f,  0.3125f,
                                              0.375f, 0.4375f, 0.5f,   0.5625f, 0.625f, 0.6875f,
                                              0.75f,  0.8125f, 0.875f, 0.9375f, 1.0f};

    static const inline std::unordered_map<char, std::vector<unsigned int>> ASCII_font = {
        {'!', std::vector<unsigned int>{8, 2, 8, 11, 8, 13, 8, 14}},
        {'"', std::vector<unsigned int>{5, 2, 5, 4, 11, 2, 11, 4}},
        {'#', std::vector<unsigned int>{7, 2, 3, 14, 13, 2, 9, 14, 2, 5, 14, 5, 2, 11, 14, 11}},
        {'$',
         std::vector<unsigned int>{2, 12, 14, 12, 14, 12, 14, 8, 14, 8, 2, 8, 2, 8, 2, 4, 2, 4, 14, 4, 8, 2, 8, 14}},
        {'%', std::vector<unsigned int>{2,  2, 2,  6,  2,  6,  6,  6,  6,  6,  6,  2,  6,  2,  2,  2,  2,  14,
                                        14, 2, 10, 10, 10, 14, 10, 14, 14, 14, 14, 14, 14, 10, 14, 10, 10, 10}},
        {'&',
         std::vector<unsigned int>{12, 14, 6,  6, 6, 6,  6, 4,  6, 4,  7, 2,  7, 2,  9, 2,  9,  2,  10, 4,  10, 4,
                                   10, 6,  10, 6, 4, 10, 4, 10, 4, 13, 4, 13, 6, 14, 6, 14, 10, 14, 10, 14, 12, 11}},
        {'\'', std::vector<unsigned int>{8, 2, 8, 4}},
        {'(', std::vector<unsigned int>{8, 2, 6, 6, 6, 6, 6, 10, 6, 10, 8, 14}},
        {')', std::vector<unsigned int>{8, 2, 10, 6, 10, 6, 10, 10, 10, 10, 8, 14}},
        {'*', std::vector<unsigned int>{8, 5, 8, 11, 6, 6, 10, 10, 6, 10, 10, 6}},
        {'+', std::vector<unsigned int>{8, 5, 8, 11, 5, 8, 11, 8}},
        {',', std::vector<unsigned int>{7, 14, 9, 14, 9, 14, 9, 12}},
        {'-', std::vector<unsigned int>{5, 8, 11, 8}},
        {'.', std::vector<unsigned int>{8, 13, 8, 14}},
        {'/', std::vector<unsigned int>{11, 2, 5, 14}},
        {'0', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 2, 14, 14, 2}},
        {'1', std::vector<unsigned int>{6, 4, 8, 2, 8, 2, 8, 14, 6, 14, 10, 14}},
        {'2',
         std::vector<unsigned int>{2, 4, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8, 2, 8, 2, 14, 2, 14, 14, 14}},
        {'3', std::vector<unsigned int>{2, 2, 14, 2, 2, 8, 14, 8, 2, 14, 14, 14, 14, 2, 14, 14}},
        {'4', std::vector<unsigned int>{2, 2, 2, 8, 14, 2, 14, 14, 2, 8, 14, 8}},
        {'5',
         std::vector<unsigned int>{14, 2, 2, 2, 2, 2, 2, 8, 2, 8, 14, 8, 14, 8, 14, 14, 14, 14, 2, 14, 2, 14, 2, 12}},
        {'6',
         std::vector<unsigned int>{14, 4, 14, 2, 14, 2, 2, 2, 2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 8, 2, 8}},
        {'7', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 2, 14}},
        {'8', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 2, 8, 14, 8}},
        {'9',
         std::vector<unsigned int>{14, 8, 2, 8, 2, 8, 2, 2, 2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 12}},
        {':', std::vector<unsigned int>{8, 3, 8, 5, 8, 13, 8, 11}},
        {';', std::vector<unsigned int>{8, 3, 8, 5, 7, 14, 9, 14, 9, 14, 9, 12}},
        {'<', std::vector<unsigned int>{14, 2, 2, 8, 2, 8, 14, 14}},
        {'=', std::vector<unsigned int>{2, 5, 14, 5, 2, 11, 14, 11}},
        {'>', std::vector<unsigned int>{2, 2, 14, 8, 14, 8, 2, 14}},
        {'?', std::vector<unsigned int>{2, 4, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 8, 8, 8, 8, 8, 11, 8, 13, 8, 14}},
        {'@', std::vector<unsigned int>{10, 10, 6,  10, 6,  10, 6,  6,  6,  6,  10, 6,  10, 6,  10, 12, 10, 12,
                                        12, 12, 12, 12, 14, 10, 14, 10, 14, 6,  14, 6,  10, 2,  10, 2,  6,  2,
                                        6,  2,  2,  6,  2,  6,  2,  10, 2,  10, 6,  14, 6,  14, 10, 14}},
        {'A', std::vector<unsigned int>{2, 14, 8, 2, 8, 2, 14, 14, 4, 10, 12, 10}},
        {'B',
         std::vector<unsigned int>{2, 2, 2, 14, 2, 2, 11, 2, 11, 2, 11, 7, 2, 7, 14, 7, 2, 14, 14, 14, 14, 14, 14, 7}},
        {'C', std::vector<unsigned int>{14, 4, 14, 2, 14, 2, 2, 2, 2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 12}},
        {'D',
         std::vector<unsigned int>{2, 2, 8, 2, 8, 2, 14, 5, 14, 5, 14, 11, 14, 11, 8, 14, 8, 14, 2, 14, 2, 14, 2, 2}},
        {'E', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 2, 8, 8, 8, 2, 14, 14, 14}},
        {'F', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 2, 8, 8, 8}},
        {'G',
         std::vector<unsigned int>{12, 4, 12, 2, 12, 2, 2, 2, 2, 2, 2, 14, 2, 14, 12, 14, 12, 14, 12, 8, 14, 8, 8, 8}},
        {'H', std::vector<unsigned int>{2, 2, 2, 14, 14, 2, 14, 14, 2, 8, 14, 8}},
        {'I', std::vector<unsigned int>{2, 2, 14, 2, 2, 14, 14, 14, 8, 2, 8, 14}},
        {'J', std::vector<unsigned int>{2, 2, 14, 2, 11, 2, 11, 11, 11, 11, 8, 14, 8, 14, 5, 14, 5, 14, 2, 11}},
        {'K', std::vector<unsigned int>{2, 2, 2, 14, 2, 8, 14, 2, 2, 8, 14, 14}},
        {'L', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14}},
        {'M', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 8, 11, 8, 11, 14, 2, 14, 2, 14, 14}},
        {'N', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 14, 14, 14, 14, 2}},
        {'O', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2}},
        {'P', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8}},
        {'Q', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 14, 14, 14, 14, 2, 14, 2, 14, 2, 2, 11, 11, 16, 16}},
        {'R', std::vector<unsigned int>{2, 14, 2, 2, 2, 2, 14, 2, 14, 2, 14, 8, 14, 8, 2, 8, 8, 8, 14, 14}},
        {'S', std::vector<unsigned int>{14, 5, 14, 2, 14, 2,  2,  2,  2, 2,  2, 8,  2, 8,
                                        14, 8, 14, 8, 14, 14, 14, 14, 2, 14, 2, 14, 2, 11}},
        {'T', std::vector<unsigned int>{2, 2, 14, 2, 8, 2, 8, 14}},
        {'U', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 2}},
        {'V', std::vector<unsigned int>{2, 2, 8, 14, 8, 14, 14, 2}},
        {'W', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 8, 5, 8, 5, 14, 14, 14, 14, 14, 2}},
        {'X', std::vector<unsigned int>{2, 2, 14, 14, 14, 2, 2, 14}},
        {'Y', std::vector<unsigned int>{2, 2, 8, 8, 8, 8, 14, 2, 8, 8, 8, 14}},
        {'Z', std::vector<unsigned int>{2, 2, 14, 2, 14, 2, 2, 14, 2, 14, 14, 14}},
        {'[', std::vector<unsigned int>{8, 0, 2, 0, 2, 0, 2, 16, 2, 16, 8, 16}},
        {'\\', std::vector<unsigned int>{5, 2, 11, 14}},
        {']', std::vector<unsigned int>{8, 0, 14, 0, 14, 0, 14, 16, 14, 16, 8, 16}},
        {'^', std::vector<unsigned int>{5, 5, 8, 2, 8, 2, 11, 5}},
        {'_', std::vector<unsigned int>{0, 16, 16, 16}},
        {'`', std::vector<unsigned int>{7, 2, 9, 5}},
        {'a', std::vector<unsigned int>{2, 10, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 10, 2, 10, 14, 8, 2, 8}},
        {'b', std::vector<unsigned int>{2, 2, 2, 14, 2, 14, 14, 14, 14, 14, 14, 8, 14, 8, 2, 8}},
        {'c', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 8, 2, 8}},
        {'d', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 14, 14, 2, 14, 8, 2, 8}},
        {'e', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 11, 14, 8, 14, 8, 2, 8, 2, 11, 14, 11}},
        {'f', std::vector<unsigned int>{8, 14, 8, 2, 8, 2, 14, 2, 14, 2, 14, 5, 2, 8, 14, 8}},
        {'g', std::vector<unsigned int>{2, 8, 2, 14, 2, 14, 14, 14, 14, 16, 14, 8, 14, 8, 2, 8, 14, 16, 2, 16}},
        {'h', std::vector<unsigned int>{2, 14, 2, 2, 2, 8, 14, 8, 14, 8, 14, 14}},
        {'i', std::vector<unsigned int>{8, 5, 8, 14, 8, 2, 8, 3}},
        {'j', std::vector<unsigned int>{8, 5, 8, 16, 8, 16, 2, 16, 8, 2, 8, 3}},
        {'k', std::vector<unsigned int>{2, 2, 2, 14, 2, 11, 14, 8, 2, 11, 14, 14}},
        {'l', std::vector<unsigned int>{8, 2, 8, 14, 8, 14, 10, 14, 8, 2, 6, 2}},
        {'m', std::vector<unsigned int>{2, 8, 2, 14, 8, 8, 8, 14, 14, 8, 14, 14, 2, 8, 14, 8}},
        {'n', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 8, 14, 8}},
        {'o', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14}},
        {'p', std::vector<unsigned int>{2, 8, 2, 16, 14, 8, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14}},
        {'q', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 16, 2, 8, 14, 8, 2, 14, 14, 14}},
        {'r', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 11, 2, 8, 14, 8}},
        {'s', std::vector<unsigned int>{2, 8, 2, 11, 14, 11, 14, 14, 2, 8, 14, 8, 2, 14, 14, 14, 2, 11, 14, 11}},
        {'t', std::vector<unsigned int>{8, 2, 8, 14, 2, 5, 14, 5, 8, 14, 14, 14}},
        {'u', std::vector<unsigned int>{2, 8, 2, 14, 14, 8, 14, 14, 2, 14, 14, 14}},
        {'v', std::vector<unsigned int>{2, 8, 8, 14, 8, 14, 14, 8}},
        {'w', std::vector<unsigned int>{2, 8, 5, 14, 5, 14, 8, 8, 8, 8, 11, 14, 11, 14, 14, 8}},
        {'x', std::vector<unsigned int>{2, 8, 14, 14, 14, 8, 2, 14}},
        {'y', std::vector<unsigned int>{2, 8, 8, 14, 6, 16, 14, 8}},
        {'z', std::vector<unsigned int>{2, 8, 14, 8, 14, 8, 2, 14, 2, 14, 14, 14}},
        {'{', std::vector<unsigned int>{10, 2, 8, 2, 8, 2, 7, 4,  7, 4,  7, 7,  7, 7,  5,  8,
                                        5,  8, 7, 9, 7, 9, 7, 12, 7, 12, 8, 14, 8, 14, 10, 14}},
        {'|', std::vector<unsigned int>{8, 0, 8, 16}},
        {'}', std::vector<unsigned int>{6,  2, 8, 2, 8, 2, 9, 4,  9, 4,  9, 7,  9, 7,  11, 8,
                                        11, 8, 9, 9, 9, 9, 9, 12, 9, 12, 8, 14, 8, 14, 6,  14}},
        {'~', std::vector<unsigned int>{2,  10, 2,  8,  2,  8,  4,  6,  4,  6, 6,  6, 6,  6,
                                        10, 10, 10, 10, 12, 10, 12, 10, 14, 8, 14, 8, 14, 6}}};
};

} // namespace mareweb

#endif // MAREWEB_TEXT_HPP