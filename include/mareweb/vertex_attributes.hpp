// vertex_attributes.hpp
#ifndef MAREWEB_VERTEX_ATTRIBUTES_HPP
#define MAREWEB_VERTEX_ATTRIBUTES_HPP

#include <cstdint>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

// Represents a single vertex attribute description
struct vertex_attribute {
  uint32_t location;
  wgpu::VertexFormat format;
  uint64_t offset;
  const char *semantic_name; // For debugging/reflection

  vertex_attribute(uint32_t loc, wgpu::VertexFormat fmt, uint64_t off, const char *name)
      : location(loc), format(fmt), offset(off), semantic_name(name) {}
};

// Holds the complete vertex layout description
class vertex_layout {
public:
  void add_attribute(const vertex_attribute &attr) {
    m_attributes.push_back(attr);
    update_stride();
  }

  [[nodiscard]] auto get_attributes() const -> const std::vector<vertex_attribute> & { return m_attributes; }

  [[nodiscard]] auto get_stride() const -> uint64_t { return m_stride; }

  [[nodiscard]] auto get_wgpu_attributes() const -> std::vector<wgpu::VertexAttribute> {
    std::vector<wgpu::VertexAttribute> attrs;
    attrs.reserve(m_attributes.size());

    for (const auto &attr : m_attributes) {
      wgpu::VertexAttribute wgpu_attr;
      wgpu_attr.format = attr.format;
      wgpu_attr.offset = attr.offset;
      wgpu_attr.shaderLocation = attr.location;
      attrs.push_back(wgpu_attr);
    }

    return attrs;
  }

private:
  std::vector<vertex_attribute> m_attributes;
  uint64_t m_stride = 0;

  void update_stride() {
    m_stride = 0;
    for (const auto &attr : m_attributes) {
      uint64_t size;
      switch (attr.format) {
      case wgpu::VertexFormat::Float32x2:
        size = 8;
        break;
      case wgpu::VertexFormat::Float32x3:
        size = 12;
        break;
      case wgpu::VertexFormat::Float32x4:
        size = 16;
        break;
      default:
        size = 0;
        break;
      }
      m_stride = std::max(m_stride, attr.offset + size);
    }
  }
};

// Common vertex layouts
namespace vertex_layouts {
inline auto pos3_norm3_tex2() -> vertex_layout {
  vertex_layout layout;
  layout.add_attribute({0, wgpu::VertexFormat::Float32x3, 0, "POSITION"});
  layout.add_attribute({1, wgpu::VertexFormat::Float32x3, 12, "NORMAL"});
  layout.add_attribute({2, wgpu::VertexFormat::Float32x2, 24, "TEXCOORD"});
  return layout;
}

inline auto pos3_norm3() -> vertex_layout {
  vertex_layout layout;
  layout.add_attribute({0, wgpu::VertexFormat::Float32x3, 0, "POSITION"});
  layout.add_attribute({1, wgpu::VertexFormat::Float32x3, 12, "NORMAL"});
  return layout;
}

inline auto pos3_tex2() -> vertex_layout {
  vertex_layout layout;
  layout.add_attribute({0, wgpu::VertexFormat::Float32x3, 0, "POSITION"});
  layout.add_attribute({1, wgpu::VertexFormat::Float32x2, 12, "TEXCOORD"});
  return layout;
}

inline auto pos3() -> vertex_layout {
  vertex_layout layout;
  layout.add_attribute({0, wgpu::VertexFormat::Float32x3, 0, "POSITION"});
  return layout;
}
} // namespace vertex_layouts

// Helper struct to store vertex data
struct vertex {
  float position[3] = {0.0f, 0.0f, 0.0f};
  float normal[3] = {0.0f, 1.0f, 0.0f};
  float texcoord[2] = {0.0f, 0.0f};

  vertex() = default;

  vertex(const float pos[3], const float norm[3] = nullptr, const float tex[2] = nullptr) {
    std::copy(pos, pos + 3, position);
    if (norm)
      std::copy(norm, norm + 3, normal);
    if (tex)
      std::copy(tex, tex + 2, texcoord);
  }
};

} // namespace mareweb

#endif // MAREWEB_VERTEX_ATTRIBUTES_HPP