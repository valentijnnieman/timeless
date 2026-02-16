# Timeless Engine - Copilot Instructions

## Project Overview
Timeless is a 2D game engine written in C++ that uses an Entity Component System (ECS) architecture. It's designed for building grid-based games with time sequencing mechanics, similar to a music sequencer where actions occur at fixed time intervals.

## Architecture & Design Principles

### Header-Only Design
- The project uses a header-only design for ease of development and flexibility during early stages
- Source code is primarily in `.hpp` files under `src/timeless/`
- This may change as the project matures toward a 1.0 release

### Entity Component System (ECS)
- Core architectural pattern for the engine
- Entities are managed through the entity system
- Components define data
- Systems process entities with specific components

### Time Sequencing System
- Central feature: objects act on a fixed time scale
- Works like a music sequencer with fixed time intervals
- Grid-based system defined in `algorithms/graph.hpp`
- `Behaviour` system: vectors of lambda functions executed in sequence
- `NpcAiSystem` handles behavior sequencing
- Supports forward and backward time traversal (undo/redo mechanics)

## Technology Stack

### Language & Standards
- C++20 (set via CMakeLists.txt)
- CMake 3.19+ for build configuration
- Cross-platform: Windows, macOS, Linux, and WebAssembly (via Emscripten)

### Key Dependencies
- **GLFW**: Window and input handling
- **GLAD**: OpenGL loading library
- **GLM**: OpenGL Mathematics library
- **FreeType**: Font rendering
- **FMOD**: Audio system (core and studio)
- **Cereal**: Serialization library
- **STB Image**: Image loading
- **FFTW**: Fast Fourier Transform (for Pontius sound system, requires float precision build)

### Build System
- CMake-based build system
- Separate configurations for native and Emscripten (WASM) builds
- Platform-specific handling for Windows, macOS, and Linux
- Example project in `example/` directory
- Resource packer utility: `timelessResourcePacker`

## Project Structure

```
src/timeless/
├── algorithm/       # Graph and pathfinding algorithms
├── components/      # ECS components
├── managers/        # Resource and state managers
├── systems/         # ECS systems (rendering, AI, animation, etc.)
├── ui/              # UI components
├── entity.hpp       # Entity definitions
├── event.hpp        # Event system
├── settings.hpp     # Configuration
├── timeless.hpp     # Main engine header
└── timer.hpp        # Timing utilities

example/             # Example application demonstrating engine features
vendor/              # Third-party dependencies
shaders/             # GLSL shader files
```

## Key Concepts & Terminology

### Behaviour System
- `Behaviour`: A vector of lambda functions representing sequential actions
- Used for AI and scripted entity actions
- Managed by `NpcAiSystem` (name subject to change)
- Supports time-based sequencing and reversal

### Graph System
- Grid-based spatial system (supports isometric grids)
- Defined in `algorithms/graph.hpp`
- Used for pathfinding and spatial organization

### Pontius Sound System
- Depends on FFTW library
- Requires FFTW built with float precision: `./configure --enable-float`
- Use `emconfigure` and `emmake` for WASM builds

## Code Style & Conventions

### General Guidelines
- Follow header-only design for now
- Keep classes relatively compact
- Refer to [Chromium C++ Dos and Don'ts](https://chromium.googlesource.com/chromium/src/+/HEAD/styleguide/c++/c++-dos-and-donts.md) for future refactoring
- Expect API changes during early development

### Naming Conventions
- Systems typically end with "System" (e.g., `NpcAiSystem`, `RenderingSystem`)
- Components are simple data structures
- Manager classes handle resource lifecycle

## Building the Project

### Native Build
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### WASM Build (Emscripten)
```bash
mkdir build && cd build
emcmake cmake -DEMSCRIPTEN=ON ..
emmake make
```

### Platform-Specific Notes
- **FMOD**: DLL/dylib files are copied post-build on Windows/macOS
- **Linux**: Requires X11 (Wayland disabled in current config)
- **WASM**: Uses Emscripten ports for GLFW3

## Example Application
- Located in `example/` directory
- Demonstrates:
  - Isometric grid setup
  - Sprite animation system
  - Behavior timeline with AI system
  - Tile rendering from spritesheets

## Development Status
- Version 0.3 (from CMakeLists.txt)
- Early stage development
- Breaking changes expected
- Not recommended for production use unless interested in the graph + time sequencing features

## Important Notes for Code Assistance
- This is a game engine in active development, expect incomplete features
- Maintain the header-only structure for now
- Consider cross-platform compatibility (Windows, macOS, Linux, WASM)
- Time sequencing and behavior systems are core features - preserve their functionality
- The engine is designed for grid-based games with time manipulation mechanics
