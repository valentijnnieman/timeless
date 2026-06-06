#pragma once
#include "timeless/entity.hpp"
#include "timeless/managers/component_manager.hpp"

// Generic scene lifecycle base.
//
// This is an engine-level concept: a Scene is just something with an
// init/update/remove lifecycle plus optional loading-screen hooks. It
// deliberately knows nothing about maps, citizens, scenarios or game
// assets (the `Resource` namespace). Game-specific behaviour belongs in
// a subclass — see Complot's `ComplotScene`.
class Scene {
public:
  // The camera entity this scene renders through. Set by the subclass'
  // owner; the engine never creates it.
  Entity main_camera;

  // Loading lifecycle flags, driven by the main loop / SceneSystem.
  bool loading = true;
  bool first_load = true;
  bool running = true;

  Scene(Entity main_cam) : main_camera(main_cam) {}
  virtual ~Scene() = default;

  virtual void init(GLFWwindow * /*window*/) {}
  virtual void update(float /*dt*/) {}
  virtual void remove() {}

  // Advance one step of initialization. Returns true when fully done.
  // Default: run init() in a single blocking step (fine for simple scenes;
  // scenes with chunked/streamed loading override this).
  virtual bool init_step(GLFWwindow *window) {
    init(window);
    return true;
  }

  // Loading-screen hooks. The engine calls these around init_step(); the
  // default is a no-op so scenes that don't draw a loading screen Just Work.
  // A subclass that wants a loading screen overrides these with its own
  // (asset-dependent) implementation.
  virtual void setup_loading_screen(GLFWwindow * /*window*/,
                                    ComponentManager & /*cm*/) {}
  virtual void remove_loading_screen() {}
};
