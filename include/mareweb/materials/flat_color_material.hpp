#ifndef MAREWEB_FLAT_COLOR_MATERIAL_HPP
#define MAREWEB_FLAT_COLOR_MATERIAL_HPP

#include "mareweb/material.hpp"
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
                 vertex_state{true, true}) // Normals enabled, texcoords disabled
  {
    // Initialize MVP matrix with identity
    auto eye = mat4::eye();
    update_uniform(0, &eye);

    // Initialize color
    update_uniform(1, &color);

    // Initialize light direction
    vec3 light_dir{1.0f, 1.0f, 1.0f};
    light_dir = normalize(light_dir);
    update_uniform(2, &light_dir);
  }

  void update_mvp(const mat4 &mvp) { update_uniform(0, &mvp); }

  void update_color(const vec4 &color) { update_uniform(1, &color); }

  void update_light_direction(const vec3 &light_dir) { update_uniform(2, &light_dir); }

private:
  static std::string get_vertex_shader() {
    return R"(
            @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;

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
                out.world_normal = normalize(in.normal);
                return out;
            }
        )";
  }

  static std::string get_fragment_shader() {
    return R"(
            @group(0) @binding(1) var<uniform> color: vec4<f32>;
            @group(0) @binding(2) var<uniform> light_direction: vec3<f32>;

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

    // Color binding
    uniform_binding color_binding;
    color_binding.binding = 1;
    color_binding.visibility = wgpu::ShaderStage::Fragment;
    color_binding.size = sizeof(vec4);

    // Light direction binding
    uniform_binding light_binding;
    light_binding.binding = 2;
    light_binding.visibility = wgpu::ShaderStage::Fragment;
    light_binding.size = sizeof(vec3);

    return {std::move(mvp_binding), std::move(color_binding), std::move(light_binding)};
  }
};

} // namespace mareweb

#endif // MAREWEB_FLAT_COLOR_MATERIAL_HPP