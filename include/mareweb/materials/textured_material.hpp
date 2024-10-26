// textured_material.hpp
#ifndef MAREWEB_TEXTURED_MATERIAL_HPP
#define MAREWEB_TEXTURED_MATERIAL_HPP

#include "mareweb/material.hpp"
#include "mareweb/texture.hpp"
#include <memory>
#include <squint/tensor.hpp>
#include <string>

namespace mareweb {
using namespace squint;

class textured_material : public material {
public:
  textured_material(wgpu::Device &device, wgpu::TextureFormat surface_format, uint32_t sample_count,
                    const char *texture_path)
      : material(device, get_vertex_shader(), get_fragment_shader(), surface_format, sample_count, get_bindings()),
        m_texture(device, texture_path) {
    // Initialize MVP matrix with identity
    auto eye = mat4::eye();
    update_uniform(0, &eye);

    // Initialize light direction
    vec3 light_dir{1.0f, 1.0f, 1.0f};
    light_dir = normalize(light_dir);
    update_uniform(1, &light_dir);
  }

  void update_mvp(const mat4 &mvp) { update_uniform(0, &mvp); }

  void update_light_direction(const vec3 &light_dir) { update_uniform(1, &light_dir); }

  [[nodiscard]] auto get_texture() const -> const texture & { return m_texture; }

private:
  texture m_texture;

  static std::string get_vertex_shader() {
    return R"(
            @group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;

            struct VertexInput {
                @location(0) position: vec3<f32>,
                @location(1) normal: vec3<f32>,
                @location(2) texcoord: vec2<f32>,
            };

            struct VertexOutput {
                @builtin(position) position: vec4<f32>,
                @location(0) world_normal: vec3<f32>,
                @location(1) texcoord: vec2<f32>,
            };

            @vertex
            fn main(in: VertexInput) -> VertexOutput {
                var out: VertexOutput;
                out.position = mvp * vec4<f32>(in.position, 1.0);
                out.world_normal = normalize(in.normal);
                out.texcoord = in.texcoord;
                return out;
            }
        )";
  }

  static std::string get_fragment_shader() {
    return R"(
            @group(0) @binding(1) var<uniform> light_direction: vec3<f32>;
            @group(0) @binding(2) var diffuse_texture: texture_2d<f32>;
            @group(0) @binding(3) var diffuse_sampler: sampler;

            @fragment
            fn main(
                @location(0) world_normal: vec3<f32>,
                @location(1) texcoord: vec2<f32>
            ) -> @location(0) vec4<f32> {
                // Sample texture
                let base_color = textureSample(diffuse_texture, diffuse_sampler, texcoord);
                
                // Calculate lighting
                let n_dot_l = max(dot(normalize(world_normal), normalize(light_direction)), 0.0);
                let ambient = 0.2;
                let diffuse = n_dot_l;
                let lighting = ambient + diffuse * 0.8; // Reduce diffuse contribution to 0.8
                
                // Combine lighting with texture
                return vec4<f32>(base_color.rgb * lighting, base_color.a);
            }
        )";
  }

  static auto get_bindings() -> std::vector<binding_resource> {
    // MVP matrix binding
    uniform_binding mvp_binding;
    mvp_binding.binding = 0;
    mvp_binding.visibility = wgpu::ShaderStage::Vertex;
    mvp_binding.size = sizeof(mat4);

    // Light direction binding
    uniform_binding light_binding;
    light_binding.binding = 1;
    light_binding.visibility = wgpu::ShaderStage::Fragment;
    light_binding.size = sizeof(vec3);

    // Texture binding
    texture_binding tex_binding;
    tex_binding.binding = 2;
    tex_binding.visibility = wgpu::ShaderStage::Fragment;
    tex_binding.texture_view = nullptr; // Will be set in constructor
    tex_binding.sample_type = wgpu::TextureSampleType::Float;
    tex_binding.view_dimension = wgpu::TextureViewDimension::e2D;

    // Sampler binding
    sampler_binding samp_binding;
    samp_binding.binding = 3;
    samp_binding.visibility = wgpu::ShaderStage::Fragment;
    samp_binding.sampler = nullptr; // Will be set in constructor
    samp_binding.type = wgpu::SamplerBindingType::Filtering;

    return {std::move(mvp_binding), std::move(light_binding), std::move(tex_binding), std::move(samp_binding)};
  }

  // Constructor helper to initialize bindings
  void init_bindings() {
    // Update the texture and sampler bindings with actual values
    update_texture(2, m_texture.get_texture_view());
    update_sampler(3, m_texture.get_sampler());
  }
};

} // namespace mareweb

#endif // MAREWEB_TEXTURED_MATERIAL_HPP