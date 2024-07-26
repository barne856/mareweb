#ifndef MAREWEB_SIMPLE_LIT_MATERIAL_HPP
#define MAREWEB_SIMPLE_LIT_MATERIAL_HPP

#include "mareweb/material.hpp"
#include <squint/fixed_tensor.hpp>
#include <string>
#include <vector>

namespace mareweb {

using namespace squint;

class simple_lit_material : public material {
public:
    simple_lit_material(wgpu::Device &device, wgpu::TextureFormat surface_format, uint32_t sample_count, 
                        const vec4 &color, const vec3 &light_direction)
        : material(device, get_vertex_shader(), get_fragment_shader(), surface_format, sample_count,
                   get_uniform_infos()) {
        auto eye = mat4::eye();
        update_uniform(0, &eye); // Initialize MVP with identity matrix
        update_uniform(1, &color);
        update_uniform(2, &light_direction);
    }

    void update_mvp(const mat4 &mvp) { update_uniform(0, &mvp); }
    void update_light_direction(const vec3 &light_direction) { update_uniform(2, &light_direction); }

private:
    static std::string get_vertex_shader() {
        return R"(
        @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;

        struct VertexOutput {
            @builtin(position) position: vec4<f32>,
            @location(0) normal: vec3<f32>,
        };

        @vertex
        fn main(
            @location(0) position: vec3<f32>,
            @location(1) normal: vec3<f32>
        ) -> VertexOutput {
            var output: VertexOutput;
            output.position = mvp * vec4<f32>(position, 1.0);
            output.normal = normal;
            return output;
        }
        )";
    }

    static std::string get_fragment_shader() {
        return R"(
        @group(0) @binding(1) var<uniform> color: vec4<f32>;
        @group(0) @binding(2) var<uniform> light_direction: vec3<f32>;

        @fragment
        fn main(
            @location(0) normal: vec3<f32>
        ) -> @location(0) vec4<f32> {
            let n = normalize(normal);
            let l = normalize(light_direction);
            let diffuse = max(dot(n, l), 0.0);
            let ambient = 0.1;
            let lighting = ambient + diffuse;
            return vec4<f32>(color.rgb * lighting, color.a);
        }
        )";
    }

    static std::vector<uniform_info> get_uniform_infos() {
        return {
            {0, sizeof(mat4), wgpu::ShaderStage::Vertex},
            {1, sizeof(vec4), wgpu::ShaderStage::Fragment},
            {2, sizeof(vec3), wgpu::ShaderStage::Fragment}
        };
    }
};

} // namespace mareweb

#endif // MAREWEB_SIMPLE_LIT_MATERIAL_HPP