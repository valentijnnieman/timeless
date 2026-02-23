# Timeless Engine

A 2D game engine written in C++ built around an **Entity Component System (ECS)** architecture with a unique time-sequencing system. Supports desktop (Windows, macOS, Linux) and **WebAssembly** builds.

![Example gif](https://github.com/valentijnnieman/timeless/blob/main/example.gif)

> ⚠️ This engine is a work in progress. Breaking changes may occur.

---

## Features

- **ECS architecture** — entities, components, and systems are cleanly separated
- **Time-sequencing (Behaviour system)** — schedule ordered sequences of actions for entities across fixed time ticks, with built-in support for going forwards *and* backwards in time
- **Isometric grid** — a built-in graph data structure for 2D tile grids with automatic neighbour calculation
- **Rendering** — OpenGL-based sprite, text, and particle rendering with support for multiple layered render passes and a full-screen shader pipeline
- **Camera system** — orthographic and perspective cameras; supports multiple cameras per scene (e.g., separate game and UI cameras)
- **Animation system** — keyframe-based animations with positional interpolation
- **Input handling** — keyboard and mouse input systems, including hover and click detection in both orthographic and perspective modes
- **Sound** — FMOD-based sound system
- **Movement system** — component-based entity movement with keyboard control support
- **WebAssembly support** — build and run in the browser via Emscripten

---

## Architecture

### Core concepts

| Concept | Description |
|---|---|
| `Entity` | A simple integer ID representing a game object |
| `Component` | Plain data structs attached to entities (e.g., `Transform`, `Sprite`, `Behaviour`) |
| `System` | Logic that processes entities with specific components (e.g., `RenderingSystem`, `NpcAiSystem`) |
| `ComponentManager` | Stores and retrieves all components |
| `WindowManager` | Manages the GLFW window and OpenGL framebuffers |

### Time-sequencing

The `NpcAiSystem` drives the time-sequencing model. Each entity can have a `Behaviour` component, which holds a list of `Instruction`s (lambda functions). The system advances through these instructions tick-by-tick at a user-defined interval. Because instructions can also be traversed in reverse, this naturally supports **undo** or **rewind** mechanics.

```cpp
std::vector<Instruction> instructions;
for (int i = 0; i < 100; i++) {
    instructions.push_back(Instruction([](Entity entity, bool reverse, float speed) {
        // do something each tick
    }));
}
TE::add_component<Behaviour>(my_entity, new Behaviour(instructions));
TE::get_system<NpcAiSystem>("NpcAiSystem")->register_entity(my_entity);
```

### Isometric grid

Nodes placed on the grid are connected to their neighbours automatically after calling `TE::calculate_grid()`. Use `graph.hpp` to traverse or query the grid.

```cpp
TE::add_component<Node>(tile, new Node(x, y, layer, tile_w, tile_h, x, 0));
// ... add all tiles ...
TE::calculate_grid(); // calculates all neighbour connections
```

---

## Dependencies

The following libraries are required. Most are included as git submodules under `vendor/`.

| Library | Purpose |
|---|---|
| [GLFW](https://www.glfw.org/) | Window creation and input |
| [GLAD](https://glad.dav1d.de/) | OpenGL function loader |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices) |
| [FreeType](https://freetype.org/) | Font rendering |
| [FMOD](https://www.fmod.com/) | Audio playback |
| [Assimp](https://github.com/assimp/assimp) | 3D model importing |
| [cereal](https://uscilab.github.io/cereal/) | Serialization |
| [stb_image](https://github.com/nothings/stb) | Image loading |

> **FMOD** is a proprietary library. You must download it separately from [fmod.com](https://www.fmod.com/download) and place it in `vendor/fmod/`.

---

## Building

### Prerequisites

- CMake ≥ 3.19
- A C++20-compatible compiler (GCC, Clang, MSVC)
- All submodules initialized: `git submodule update --init --recursive`
- FMOD SDK placed in `vendor/fmod/` (see above)

### Desktop (Linux / macOS / Windows)

```bash
cmake -B build
cmake --build build
```

On **Linux**, X11 is used by default (Wayland support can be enabled in `CMakeLists.txt`).

On **Windows**, FMOD DLLs are automatically copied to the build output directory.

### WebAssembly (Emscripten)

```bash
emcmake cmake -B build-wasm -DEMSCRIPTEN=ON
emmake cmake --build build-wasm
```

---

## Quick Start

```cpp
#include "timeless/timeless.hpp"

int main() {
    TE::init();

    // Create systems
    TE::create_system<RenderingSystem>("Renderer", new RenderingSystem());
    TE::create_system<NpcAiSystem>("AI", new NpcAiSystem());

    // Create a camera
    Entity cam = create_entity();
    TE::add_component<Camera>(cam, new Camera(glm::vec3(0.0f, 0.0f, 10.0f)));
    TE::get_system<RenderingSystem>("Renderer")->register_camera(cam);

    // Create an entity with a sprite and a timed behaviour
    Entity e = create_entity();
    TE::add_component<Transform>(e, new Transform(glm::vec3(0, 0, 0), 0.f, 32.f, 32.f));
    TE::add_component<Behaviour>(e, new Behaviour({
        Instruction([](Entity ent, bool reverse, float speed) { /* tick 0 */ }),
        Instruction([](Entity ent, bool reverse, float speed) { /* tick 1 */ }),
    }));
    TE::get_system<RenderingSystem>("Renderer")->register_entity(e);
    TE::get_system<NpcAiSystem>("AI")->register_entity(e);

    auto wm = TE::get_window_manager();
    while (!glfwWindowShouldClose(wm->window) && wm->running) {
        TE::loop([&](GLFWwindow* window, ComponentManager& cm, WindowManager& wm) {
            TE::get_system<NpcAiSystem>("AI")->update(cm);
            TE::get_system<RenderingSystem>("Renderer")->render(cm, 1920, 1080, 1.0f, 0);
            glfwSwapBuffers(window);
            glfwPollEvents();
        });
    }

    TE::cleanup();
    return 0;
}
```

---

## Example

The `example/` directory contains a complete program that:

- Sets up a 20×20 isometric tile grid from a spritesheet
- Animates tiles using the `AnimationSystem`
- Sequences per-tile behaviours with the `NpcAiSystem`
- Uses two cameras (game world + UI) and two render layers

Build it alongside the engine:

```bash
cmake -B build && cmake --build build
./build/example/example
```

---

## Should I use this?

Timeless is an early-stage engine tailored for games that need **time-sequenced, grid-based mechanics**. If you are interested in the time-rewinding / sequencer concept, want to build isometric grid games, or want to contribute — you're welcome!

For general-purpose 2D game development, a more mature engine may be a better fit today.

---

## Contributing

Contributions, bug reports, and feature requests are welcome. Feel free to open an issue or a pull request.

---

## License

This project does not currently have an explicit license. Please contact the author before using it in your own projects.

