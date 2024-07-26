#ifndef MAREWEB_PRIMITIVE_MESHE_HPP
#define MAREWEB_PRIMITIVE_MESHE_HPP

#include "mareweb/mesh.hpp"
#include "squint/fixed_tensor.hpp"
#include "squint/quantity.hpp"

namespace mareweb {

using vec3 = squint::vec3;
using length = squint::units::length;

class triangle_mesh : public mesh {
public:
  triangle_mesh(wgpu::Device &device, const vec3 &v1, const vec3 &v2, const vec3 &v3)
      : mesh(device, wgpu::PrimitiveTopology::TriangleList,
             {v1[0], v1[1], v1[2], v2[0], v2[1], v2[2], v3[0], v3[1], v3[2]}) {}
};

class rectangle_mesh : public mesh {
public:
  rectangle_mesh(wgpu::Device &device, length width, length height)
      : mesh(device, wgpu::PrimitiveTopology::TriangleList,
             {-width.value() / 2, -height.value() / 2, 0.0f, width.value() / 2, -height.value() / 2, 0.0f,
              width.value() / 2, height.value() / 2, 0.0f, -width.value() / 2, height.value() / 2, 0.0f},
             {0, 1, 2, 2, 3, 0}) {}
};

class cube_mesh : public mesh {
public:
  cube_mesh(wgpu::Device &device, length size)
      : mesh(device, wgpu::PrimitiveTopology::TriangleList,
             {// Front face
              -size.value() / 2, -size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2,
              size.value() / 2, size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2,
              size.value() / 2, size.value() / 2,
              // Back face
              -size.value() / 2, -size.value() / 2, -size.value() / 2, size.value() / 2, -size.value() / 2,
              -size.value() / 2, size.value() / 2, size.value() / 2, -size.value() / 2, -size.value() / 2,
              size.value() / 2, -size.value() / 2},
             {
                 0, 1, 2, 2, 3, 0, // Front face
                 1, 5, 6, 6, 2, 1, // Right face
                 5, 4, 7, 7, 6, 5, // Back face
                 4, 0, 3, 3, 7, 4, // Left face
                 3, 2, 6, 6, 7, 3, // Top face
                 4, 5, 1, 1, 0, 4  // Bottom face
             }) {}
};

class line_mesh : public mesh {
public:
  line_mesh(wgpu::Device &device, const vec3 &start, const vec3 &end)
      : mesh(device, wgpu::PrimitiveTopology::LineList, {start[0], start[1], start[2], end[0], end[1], end[2]}) {}
};

class point_mesh : public mesh {
public:
  point_mesh(wgpu::Device &device, const vec3 &point)
      : mesh(device, wgpu::PrimitiveTopology::PointList, {point[0], point[1], point[2]}) {}
};

} // namespace mareweb
#endif