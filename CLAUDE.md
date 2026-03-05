# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

**Timeless** is a C++20 game engine built around an Entity Component System (ECS) with a unique time-sequencing system. Its defining feature is the `NpcAiSystem`, which drives tick-based entity behaviors that can play forward *and* backward — enabling rewind/replay mechanics. It is used as a submodule by the Complot detective game.

## Build commands

### Prerequisites
```bash
git submodule update --init --recursive
# Also place FMOD SDK in vendor/fmod/ (download from fmod.com)
```

### Desktop (Linux/macOS/Windows)
```bash
cmake -B build
cmake --build build
```

### WebAssembly
```bash
emcmake cmake -B build-wasm -DEMSCRIPTEN=ON
emmake cmake --build build-wasm
```

The engine builds as a **static library** (`timeless`). There is a separate `timelessResourcePacker` executable for packing assets into a binary file.

## Architecture

### ECS pattern

- `Entity` — a `uint32_t` ID from `create_entity()`
- `Component` — data structs inheriting from `Component` base class
- `System` — logic processing registered entities; extend `System` base class
- `ComponentManager` — global store; access via `TE::cm`

```cpp
Entity e = create_entity();
TE::add_component<Transform>(e, new Transform(...));
TE::get_component<Transform>(e)->position = ...;
TE::get_system<RenderingSystem>("Renderer")->register_entity(e);
```

`TE::` is the global engine namespace defined in `include/timeless/timeless.hpp`. Key globals:
- `TE::cm` — ComponentManager
- `TE::tm` — TimerManager
- `TE::mis` — MouseInputSystem
- `TE::wm` — WindowManager
- `TE::grid` — Grid (pathfinding)
- `TE::systems` — map of named System instances

### Time-sequencing (`NpcAiSystem` + `Behaviour`)

The `Behaviour` component holds a `std::vector<Instruction>` — one lambda per tick. `NpcAiSystem` advances `main_index` each timer interval and calls `instruction.run(entity, reverse, speed)`. Because the index can go forward or backward, time rewind comes for free.

- 1 tick = 5 in-game minutes; default `TESettings::MAX_TICKS = 200` (~16 hours)
- Speed and direction are controlled on the `NpcAiSystem` instance directly
- Pre-compute all instructions upfront (fill the entire vector); do not dynamically push during playback

### Rendering

Multiple named `RenderingSystem` instances can coexist with different cameras (e.g., world camera + UI camera). Key render systems:
- `RenderingSystem` — sprites and quads (instanced)
- `AnimationSystem` — keyframe bone animation
- Post-processing via framebuffers → shaders in `shaders/`

`render(cm, width, height, zoom, layer)` — call once per system per frame.

### Grid / pathfinding

`Node` components define tile positions. After adding all tiles, call `TE::calculate_grid()` to build neighbor connections. Query via `TE::grid`. `LocationType` enum on nodes drives NPC behavior slot assignment (home, office, food, etc.).

### Input

- `MouseInputListener<T>` — attach to entity; register with `TE::mis`
- `KeyboardInputListener` — attach to entity; register with `KeyboardInputSystem`
- `TE::clicked_on(event, entity, zoom)` — orthographic hit test
- `TE::clicked_on_perspective(event, entity)` — perspective hit test

### Event system

Custom named events dispatched via:
```cpp
event_sys->create_event<MyData>(TE::cm, "EventName", &data);
```
Entities subscribe via `EventListener<MyData>` component.

## Key files

| File | Purpose |
|---|---|
| `include/timeless/timeless.hpp` | Engine API facade (`TE::` namespace, init/loop/cleanup) |
| `include/timeless/entity.hpp` | Entity, Component, System base classes; ComponentManager |
| `include/timeless/settings.hpp` | `TESettings` namespace (screen size, zoom, MAX_TICKS) |
| `include/timeless/systems/npc_ai_system.hpp` | Tick simulation with forward/reverse playback |
| `include/timeless/systems/rendering_system.hpp` | OpenGL instanced rendering |
| `include/timeless/algorithm/graph.hpp` | Grid, Node, LocationType, pathfinding |
| `include/timeless/components/behaviour.hpp` | `Behaviour` and `Instruction` types |
| `example/main.cpp` | Example (currently broken — do not use as reference) |
| `CMakeLists.txt` | Build config; add source files here when creating new systems |

## Settings

Global engine configuration lives in `TESettings` (not runtime-mutable):
- `MAX_TICKS` — total ticks in a time sequence (default 200)
- Screen dimensions, zoom, fullscreen, thread count

## Dependencies

Most are git submodules under `vendor/`: GLFW, GLAD, GLM, FreeType, Assimp, cereal, stb_image. **FMOD** (audio) must be downloaded separately from fmod.com and placed in `vendor/fmod/`.
