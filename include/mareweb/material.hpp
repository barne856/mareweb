#ifndef MAREWEB_MATERIAL_HPP
#define MAREWEB_MATERIAL_HPP

#include "mareweb/buffer.hpp"
#include "mareweb/pipeline.hpp"
#include "mareweb/shader.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace mareweb {

// Base binding info with common properties
struct binding_info {
  uint32_t binding;
  wgpu::ShaderStage visibility;
};

// Specific binding types
struct uniform_binding : binding_info {
  size_t size;
};

struct texture_binding : binding_info {
  wgpu::TextureView texture_view;
  wgpu::TextureSampleType sample_type;
  wgpu::TextureViewDimension view_dimension;
};

struct sampler_binding : binding_info {
  wgpu::Sampler sampler;
  wgpu::SamplerBindingType type;
};

struct storage_binding : binding_info {
  wgpu::Buffer buffer;
  size_t size;
  wgpu::BufferBindingType type;
};

// Use variant to store different binding types
using binding_resource = std::variant<uniform_binding, texture_binding, sampler_binding, storage_binding>;

struct pipeline_key {
  wgpu::PrimitiveTopology topology;
  wgpu::IndexFormat strip_index_format;
  wgpu::FrontFace front_face;
  wgpu::CullMode cull_mode;

  bool operator==(const pipeline_key &other) const {
    return topology == other.topology && strip_index_format == other.strip_index_format &&
           front_face == other.front_face && cull_mode == other.cull_mode;
  }
};

struct pipeline_key_hash {
  std::size_t operator()(const pipeline_key &k) const {
    std::size_t h1 = std::hash<int>()(static_cast<int>(k.topology));
    std::size_t h2 = std::hash<int>()(static_cast<int>(k.strip_index_format));
    std::size_t h3 = std::hash<int>()(static_cast<int>(k.front_face));
    std::size_t h4 = std::hash<int>()(static_cast<int>(k.cull_mode));
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
  }
};

struct vertex_requirements {
  bool needs_position = true; // Always required
  bool needs_normal = false;
  bool needs_texcoord = false;
  bool needs_color = false;

  // Check if a vertex state satisfies these requirements
  bool is_satisfied_by(const vertex_state &state) const {
    // Position is always required and always present
    return (!needs_normal || state.has_normals) && (!needs_texcoord || state.has_texcoords) &&
           (!needs_color || state.has_colors);
  }

  // Helper to create common requirement sets
  static vertex_requirements positions_only() { return vertex_requirements{}; }

  static vertex_requirements with_normals() { return vertex_requirements{true, true, false, false}; }

  static vertex_requirements with_texcoords() { return vertex_requirements{true, false, true, false}; }

  static vertex_requirements with_normals_and_texcoords() { return vertex_requirements{true, true, true, false}; }
};

namespace uniform_locations {
constexpr uint32_t MVP_MATRIX = 0;
constexpr uint32_t NORMAL_MATRIX = 1;
// Add more uniform locations as needed
} // namespace uniform_locations

class material {
public:
  material(wgpu::Device &device, const std::string &vertex_shader_source, const std::string &fragment_shader_source,
           wgpu::TextureFormat surface_format, uint32_t sample_count, const std::vector<binding_resource> &bindings,
           const vertex_requirements &requirements);

  void bind(wgpu::RenderPassEncoder &pass_encoder, const wgpu::PrimitiveState &primitive_state,
            const vertex_state &mesh_vertex_state);
  void update_uniform(uint32_t binding, const void *data);
  void update_texture(uint32_t binding, wgpu::TextureView texture_view);
  void update_sampler(uint32_t binding, wgpu::Sampler sampler);
  void update_instance_buffer(wgpu::Buffer buffer, size_t size);
  [[nodiscard]] const vertex_requirements &get_requirements() const { return m_requirements; }

protected:
  wgpu::Device &get_device() { return m_device; }
  auto get_or_create_pipeline(const wgpu::PrimitiveState &primitive_state,
                              const vertex_state &mesh_vertex_state) -> pipeline &;

private:
  wgpu::Device m_device;
  std::string m_vertex_shader_source;
  std::string m_fragment_shader_source;
  wgpu::TextureFormat m_surface_format;
  uint32_t m_sample_count;
  std::vector<binding_resource> m_bindings;
  vertex_requirements m_requirements;

  std::unique_ptr<shader> m_vertex_shader;
  std::unique_ptr<shader> m_fragment_shader;
  std::unordered_map<pipeline_key, std::unique_ptr<pipeline>, pipeline_key_hash> m_pipelines;
  std::unordered_map<uint32_t, std::unique_ptr<uniform_buffer>> m_uniform_buffers;
  std::unordered_map<uint32_t, size_t> m_uniform_sizes;

  void create_shaders();
  void create_buffers();
  auto create_bind_group_layout_entries() const -> std::vector<wgpu::BindGroupLayoutEntry>;
  auto create_bind_group_entries() const -> std::vector<wgpu::BindGroupEntry>;
};

} // namespace mareweb

#endif // MAREWEB_MATERIAL_HPP