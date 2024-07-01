# Mareweb Library: Triangle Example Documentation

This document provides a detailed explanation of the triangle example, demonstrating how to use the Mareweb library to create a simple graphics application.

## 1. Overview

The Mareweb library is a C++ framework for creating graphics applications using WebGPU. It provides an entity-component system architecture with a rendering pipeline. The triangle example demonstrates how to create a basic application that renders a single triangle.

## 2. Main Components

### 2.1 Application (`mareweb::application`)

The `application` class is the core of the Mareweb library. It manages the application lifecycle, including initialization, main loop, and cleanup.

```cpp
mareweb::application &app = mareweb::application::get_instance();
app.initialize();
```

The application is a singleton, accessed via `get_instance()`. The `initialize()` method sets up the necessary systems (like SDL and WebGPU).

### 2.2 Renderer (`mareweb::renderer`)

The renderer class handles the rendering pipeline. In this example, we create a custom `main_renderer` that inherits from `mareweb::renderer`:

```cpp
class main_renderer : public mareweb::renderer {
public:
  main_renderer(wgpu::Device &device, wgpu::Surface surface, SDL_Window *window,
                const mareweb::renderer_properties &properties)
      : renderer(device, surface, window, properties) {
    set_clear_color({0.05f, 0.05f, 0.05f, 1.0f});
    create_object<triangle>(this);
  }
};
```

The `main_renderer` constructor sets up the rendering properties and creates a `triangle` object.

### 2.3 Entity System (`mareweb::entity`)

Mareweb uses an entity system where game objects inherit from `mareweb::entity`. In this example, we define a `triangle` entity:

```cpp
class triangle : public mareweb::entity<triangle> {
  public:
  triangle(mareweb::renderer* rend) : rend(rend){
    // ... initialization code ...
    attach_system<triangle_render_system>();
  }
  // ... member variables ...
};
```

The `triangle` class inherits from `mareweb::entity<triangle>`, using the Curiously Recurring Template Pattern (CRTP). This allows the entity to have type-safe systems attached to it.

### 2.4 Render System (`mareweb::render_system`)

The render system defines how an entity should be rendered. In this example, we create a `triangle_render_system`:

```cpp
template <typename T>
class triangle_render_system : public mareweb::render_system<T> {
public:
  void render(float dt, T &ent) override {
    ent.rend->draw_mesh(*ent.mesh.get(), *ent.material.get());
  }
};
```

This system is responsible for drawing the triangle mesh using the associated material.

## 3. Initialization and Setup

### 3.1 Application Initialization

```cpp
mareweb::application &app = mareweb::application::get_instance();
app.initialize();
```

This code initializes the Mareweb application.

### 3.2 Renderer Properties

```cpp
mareweb::renderer_properties props = {
  .width = 800,
  .height = 600,
  .title = "Triangle",
  .fullscreen = false,
  .resizable = true,
  .present_mode = wgpu::PresentMode::Fifo,
  .sample_count = 4
};
```

These properties define the initial state of the renderer, including window size, title, and rendering options.

### 3.3 Creating the Renderer

```cpp
app.create_renderer<main_renderer>(props);
```

This line creates an instance of our custom `main_renderer` with the specified properties.

## 4. Entity and Mesh Creation

Inside the `triangle` constructor:

```cpp
std::vector<float> vertices = {0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f};
mesh = rend->create_mesh(vertices);
```

This code defines the vertices of the triangle and creates a mesh using these vertices.

## 5. Shader Creation

The `triangle` constructor also defines vertex and fragment shaders:

```cpp
const char *vertex_shader_source = R"(
  @vertex
  fn main(@location(0) position: vec3<f32>) -> @builtin(position) vec4<f32> {
    return vec4<f32>(position, 1.0);
  }
)";

const char *fragment_shader_source = R"(
  @fragment
  fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(1.0, 0.1, 0.05, 1.0);
  }
)";

material = rend->create_material(vertex_shader_source, fragment_shader_source);
```

These shaders define how the triangle vertices are transformed and colored.

## 6. System Attachment

```cpp
attach_system<triangle_render_system>();
```

This line attaches the `triangle_render_system` to the `triangle` entity, defining how it should be rendered.

## 7. Main Loop

```cpp
app.run();
```

This starts the main application loop, which handles events, updates entities, and renders frames.

## 8. Rendering Process

The rendering process is managed by the Mareweb library:

1. The `application` class runs the main loop.
2. For each frame, it calls the `update` and `render` methods on the `main_renderer`.
3. The `main_renderer` processes its child objects (in this case, the `triangle`).
4. The `triangle_render_system` attached to the `triangle` entity is called, which draws the mesh.

## Conclusion

This example demonstrates the core concepts of the Mareweb library:
- Application and renderer setup
- Entity creation
- Mesh and shader definition
- Render system attachment and usage

By following this pattern, you can create more complex applications with multiple entities and custom rendering logic.