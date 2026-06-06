#pragma once
#include "timeless/scene.hpp"
#include "timeless/settings.hpp"
#include "timeless/systems/system.hpp"
#include <algorithm>
#include <memory>
#include <vector>

// Owns the set of scenes and performs (deferred) transitions between them.
// Engine-level: it only ever touches the generic `Scene` interface.
class SceneSystem : public System {
private:
  std::vector<std::shared_ptr<Scene>> scenes;
  std::shared_ptr<Scene> current_scene;
  bool transition_pending = false;
  int pending_index = -1; // -1 means "next", >= 0 means specific index

public:
  void add_scene(std::shared_ptr<Scene> scene) { scenes.push_back(scene); };

  void set_current_scene(std::shared_ptr<Scene> scene) {
    current_scene = scene;
  };

  std::shared_ptr<Scene> get_current_scene() { return current_scene; };

  // Safe to call from event callbacks — defers the actual switch to apply_pending().
  void next_scene() {
    transition_pending = true;
    pending_index = -1;
  };

  void load_scene(int index) {
    transition_pending = true;
    pending_index = index;
  }

  // Call once per frame at the top of the game loop, before any scene logic.
  void apply_pending() {
    if (!transition_pending) return;
    transition_pending = false;

    TESettings::ZOOM = 1.0f;
    current_scene->remove();

    if (pending_index >= 0) {
      current_scene = scenes[pending_index];
    } else {
      auto it = find(scenes.begin(), scenes.end(), current_scene);
      int index = it - scenes.begin();
      current_scene = scenes[index + 1];
    }
    pending_index = -1;
  }
};
