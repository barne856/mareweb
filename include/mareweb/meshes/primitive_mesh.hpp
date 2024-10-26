#ifndef MAREWEB_PRIMITIVE_MESH_HPP
#define MAREWEB_PRIMITIVE_MESH_HPP

#include "mareweb/mesh.hpp"
#include "squint/quantity.hpp"
#include "squint/tensor.hpp"

namespace mareweb {

using vec3 = squint::vec3;
using length = squint::length;

class triangle_mesh : public mesh {
public:
  triangle_mesh(wgpu::Device &device, const squint::vec3_t<length> &v1, const squint::vec3_t<length> &v2,
                const squint::vec3_t<length> &v3)
      : mesh(device, get_primitive_state(),
             {v1[0].value(), v1[1].value(), v1[2].value(), v2[0].value(), v2[1].value(), v2[2].value(), v3[0].value(),
              v3[1].value(), v3[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class square_mesh : public mesh {
public:
  square_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(),
             {-size.value() / 2, -size.value() / 2, 0.0f, size.value() / 2, -size.value() / 2, 0.0f, size.value() / 2,
              size.value() / 2, 0.0f, -size.value() / 2, size.value() / 2, 0.0f},
             {0, 1, 2, 2, 3, 0}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }
};

class cube_mesh : public mesh {
public:
  cube_mesh(wgpu::Device &device, length size)
      : mesh(device, get_primitive_state(),
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

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::TriangleList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::Back;
    return state;
  }
};

class line_mesh : public mesh {
public:
  line_mesh(wgpu::Device &device, const squint::vec3_t<length> &start, const squint::vec3_t<length> &end)
      : mesh(device, get_primitive_state(),
             {start[0].value(), start[1].value(), start[2].value(), end[0].value(), end[1].value(), end[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::LineList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

class point_mesh : public mesh {
public:
  point_mesh(wgpu::Device &device, const squint::vec3_t<length> &point)
      : mesh(device, get_primitive_state(), {point[0].value(), point[1].value(), point[2].value()}) {}

private:
  static auto get_primitive_state() -> wgpu::PrimitiveState {
    wgpu::PrimitiveState state;
    state.topology = wgpu::PrimitiveTopology::PointList;
    state.stripIndexFormat = wgpu::IndexFormat::Undefined;
    state.frontFace = wgpu::FrontFace::CCW;
    state.cullMode = wgpu::CullMode::None;
    return state;
  }
};

} // namespace mareweb
#endif // MAREWEB_PRIMITIVE_MESH_HPP