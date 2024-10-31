#ifndef MAREWEB_VERTEX_ATTRIBUTES_HPP
#define MAREWEB_VERTEX_ATTRIBUTES_HPP

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <webgpu/webgpu_cpp.h>

namespace mareweb {

// Define attribute locations as constants to ensure consistency
namespace attribute_locations {
constexpr uint32_t POSITION = 0;
constexpr uint32_t NORMAL = 1;
constexpr uint32_t TEXCOORD = 2;
constexpr uint32_t COLOR = 3;
// Add more attribute locations as needed
} // namespace attribute_locations

// Represents a single vertex attribute description
struct vertex_attribute {
  uint32_t location;
  wgpu::VertexFormat format;
  uint64_t offset;
  std::string semantic_name;
  size_t size;

  vertex_attribute(uint32_t loc, wgpu::VertexFormat fmt, uint64_t off, const char *name)
      : location(loc), format(fmt), offset(off), semantic_name(name) {
    switch (fmt) {
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
  }
};

// Vertex data structure that can hold all possible attributes
struct vertex {
  float position[3] = {0.0f, 0.0f, 0.0f};    // Always present
  float normal[3] = {0.0f, 1.0f, 0.0f};      // Optional
  float texcoord[2] = {0.0f, 0.0f};          // Optional
  float color[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Optional

  vertex() = default;

  vertex(const float pos[3], const float norm[3] = nullptr, const float tex[2] = nullptr,
         const float col[4] = nullptr) {
    std::copy(pos, pos + 3, position);
    if (norm)
      std::copy(norm, norm + 3, normal);
    if (tex)
      std::copy(tex, tex + 2, texcoord);
    if (col)
      std::copy(col, col + 4, color);
  }
};

// Holds complete vertex layout description with flags for included attributes
class vertex_layout {
public:
  vertex_layout() = default;

  vertex_layout clone() const {
    vertex_layout new_layout;
    new_layout.m_attributes = m_attributes;
    new_layout.m_stride = m_stride;
    return new_layout;
  }

  void add_attribute(vertex_attribute attr) {
    // Ensure proper alignment and ordering
    attr.offset = m_stride;
    m_stride = attr.offset + attr.size;
    // Align to 4 bytes
    m_stride = (m_stride + 3) & ~3;

    m_attributes.push_back(attr);
  }

  // Helper methods to check which attributes are present
  bool has_positions() const { return true; } // Always true
  bool has_normals() const { return has_attribute(attribute_locations::NORMAL); }
  bool has_texcoords() const { return has_attribute(attribute_locations::TEXCOORD); }
  bool has_colors() const { return has_attribute(attribute_locations::COLOR); }

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

  bool has_attribute(uint32_t location) const {
    return std::any_of(m_attributes.begin(), m_attributes.end(),
                       [location](const vertex_attribute &attr) { return attr.location == location; });
  }

  void update_stride() {
    m_stride = 0;
    for (const auto &attr : m_attributes) {
      m_stride = std::max(m_stride, attr.offset + attr.size);
    }
    // Ensure stride is aligned to 4 bytes
    m_stride = (m_stride + 3) & ~3;
  }
};

// Factory functions for common vertex layouts
namespace vertex_layouts {
inline auto create_layout() -> vertex_layout {
  vertex_layout layout;
  layout.add_attribute({attribute_locations::POSITION, wgpu::VertexFormat::Float32x3, 0, "POSITION"});
  return layout;
}

inline auto with_normals(vertex_layout layout) -> vertex_layout {
  // Next attribute starts at the current stride
  uint64_t offset = layout.get_stride();
  layout.add_attribute({attribute_locations::NORMAL, wgpu::VertexFormat::Float32x3, offset, "NORMAL"});
  return layout;
}

inline auto with_texcoords(vertex_layout layout) -> vertex_layout {
  uint64_t offset = layout.get_stride();
  layout.add_attribute({attribute_locations::TEXCOORD, wgpu::VertexFormat::Float32x2, offset, "TEXCOORD"});
  return layout;
}

inline auto with_colors(vertex_layout layout) -> vertex_layout {
  uint64_t offset = layout.get_stride();
  layout.add_attribute({attribute_locations::COLOR, wgpu::VertexFormat::Float32x4, offset, "COLOR"});
  return layout;
}

// Convenience functions for common combinations
inline auto pos3() -> vertex_layout { return create_layout(); }

inline auto pos3_norm3() -> vertex_layout {
  vertex_layout layout = create_layout();
  layout.add_attribute({attribute_locations::NORMAL, wgpu::VertexFormat::Float32x3,
                        0, // Will be adjusted by add_attribute
                        "NORMAL"});
  return layout;
}

inline auto pos3_tex2() -> vertex_layout { return with_texcoords(create_layout()); }

inline auto pos3_norm3_tex2() -> vertex_layout {
  vertex_layout layout = pos3_norm3();
  layout.add_attribute({attribute_locations::TEXCOORD, wgpu::VertexFormat::Float32x2,
                        0, // Will be adjusted by add_attribute
                        "TEXCOORD"});
  return layout;
}

inline auto pos3_norm3_color4() -> vertex_layout { return with_colors(with_normals(create_layout())); }

inline auto pos3_norm3_tex2_color4() -> vertex_layout {
  return with_colors(with_texcoords(with_normals(create_layout())));
}

} // namespace vertex_layouts

} // namespace mareweb

#endif // MAREWEB_VERTEX_ATTRIBUTES_HPP