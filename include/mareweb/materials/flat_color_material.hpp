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
      : material(device, get_vertex_shader(), get_fragment_shader(), surface_format, sample_count,
                 get_uniform_infos()) {
    auto eye = mat4::eye();
    update_uniform(0, &eye); // Initialize MVP with identity matrix
    update_uniform(1, &color);
  }

  void update_mvp(const mat4 &mvp) { update_uniform(0, &mvp); }

private:
  static std::string get_vertex_shader() {
    return R"(
            @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;

            struct VertexOutput {
                @builtin(position) position: vec4<f32>,
            };

            @vertex
            fn main(@location(0) position: vec3<f32>) -> VertexOutput {
                var output: VertexOutput;
                output.position = mvp * vec4<f32>(position, 1.0);
                return output;
            }
        )";
  }

  static std::string get_fragment_shader() {
    return R"(
            @group(0) @binding(1) var<uniform> color: vec4<f32>;

            @fragment
            fn main() -> @location(0) vec4<f32> {
                return color;
            }
        )";
  }

  static std::vector<uniform_info> get_uniform_infos() {
    return {{0, sizeof(mat4), wgpu::ShaderStage::Vertex}, {1, sizeof(vec4), wgpu::ShaderStage::Fragment}};
  }
};

} // namespace mareweb

#endif // MAREWEB_FLAT_COLOR_MATERIAL_HPP