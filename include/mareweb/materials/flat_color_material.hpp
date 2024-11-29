#ifndef MAREWEB_FLAT_COLOR_MATERIAL_HPP
#define MAREWEB_FLAT_COLOR_MATERIAL_HPP

#include "mareweb/material.hpp"
#include "squint/tensor/tensor_types.hpp"
#include <squint/tensor.hpp>
#include <string>
#include <vector>

namespace mareweb {
using namespace squint;

class flat_color_material : public material {
public:
  flat_color_material(wgpu::Device &device, wgpu::TextureFormat surface_format, uint32_t sample_count,
                      const vec4 &color)
      : material(device, get_vertex_shader(), get_fragment_shader(), surface_format, sample_count, get_bindings(),
                 vertex_requirements::with_normals()) {
    // Initialize color
    update_color(color);

    // Initialize light direction
    vec3 light_dir{1.0f, 1.0f, 1.0f};
    light_dir = normalize(light_dir);
    update_light_direction(light_dir);
  }

  void update_color(const vec4 &color) { update_uniform(2, &color); }

  void update_light_direction(const vec3 &light_dir) {
    // In WGSL (column-major), vec3 needs to be stored as vec4 (16 bytes)
    vec4 light_dir_padded{light_dir[0], light_dir[1], light_dir[2], 0.0f};
    update_uniform(3, &light_dir_padded);
  }

private:
  static std::string get_vertex_shader() {
    return R"(
            @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;
            @group(0) @binding(1) var<uniform> normal_matrix: mat3x3<f32>;

            struct VertexInput {
                @location(0) position: vec3<f32>,
                @location(1) normal: vec3<f32>,
            };

            struct VertexOutput {
                @builtin(position) position: vec4<f32>,
                @location(0) world_normal: vec3<f32>,
            };

            @vertex
            fn main(in: VertexInput) -> VertexOutput {
                var out: VertexOutput;
                out.position = mvp * vec4<f32>(in.position, 1.0);
                out.world_normal = normalize(normal_matrix * in.normal);
                return out;
            }
        )";
  }

  static std::string get_fragment_shader() {
    return R"(
            @group(0) @binding(2) var<uniform> color: vec4<f32>;
            @group(0) @binding(3) var<uniform> light_direction: vec3<f32>;

            @fragment
            fn main(@location(0) world_normal: vec3<f32>) -> @location(0) vec4<f32> {
                // Calculate lighting
                let n_dot_l = max(dot(normalize(world_normal), normalize(light_direction)), 0.0);
                let ambient = 0.2;
                let diffuse = n_dot_l;
                let lighting = ambient + diffuse * 0.8;
                
                // Combine lighting with base color
                return vec4<f32>(color.rgb * lighting, color.a);
            }
        )";
  }

  static auto get_bindings() -> std::vector<binding_resource> {
    // MVP matrix binding
    uniform_binding mvp_binding;
    mvp_binding.binding = 0;
    mvp_binding.visibility = wgpu::ShaderStage::Vertex;
    mvp_binding.size = sizeof(mat4);

    // Normal matrix binding
    uniform_binding normal_matrix_binding;
    normal_matrix_binding.binding = 1;
    normal_matrix_binding.visibility = wgpu::ShaderStage::Vertex;
    // In WGSL (column-major), mat3x3 needs to be stored as 3 columns of vec4 (3 * 16 = 48 bytes)
    normal_matrix_binding.size = sizeof(mat4x3);

    // Color binding
    uniform_binding color_binding;
    color_binding.binding = 2;
    color_binding.visibility = wgpu::ShaderStage::Fragment;
    color_binding.size = sizeof(vec4);

    // Light direction binding
    uniform_binding light_binding;
    light_binding.binding = 3;
    light_binding.visibility = wgpu::ShaderStage::Fragment;
    // In WGSL (column-major), vec3 needs to be stored as vec4 (16 bytes)
    light_binding.size = sizeof(vec4);

    return {
        std::move(mvp_binding),
        std::move(normal_matrix_binding),
        std::move(color_binding),
        std::move(light_binding),
    };
  }
};

} // namespace mareweb

#endif // MAREWEB_FLAT_COLOR_MATERIAL_HPP