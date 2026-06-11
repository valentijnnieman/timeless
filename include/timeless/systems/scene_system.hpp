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

  // Switch to a specific scene instance, registering it first if it's new (e.g.
  // a freshly-built scene for a "restart"). Deferred to apply_pending(), which
  // calls remove() on the outgoing scene before swapping — so it's safe to call
  // from inside that scene's own update()/callbacks.
  void switch_to(std::shared_ptr<Scene> scene) {
    auto it = std::find(scenes.begin(), scenes.end(), scene);
    if (it == scenes.end()) {
      scenes.push_back(scene);
      it = scenes.end() - 1;
    }
    transition_pending = true;
    pending_index = static_cast<int>(it - scenes.begin());
  }

  // Drop every registered scene except the current one. A scene that restarts
  // itself via switch_to(fresh) keeps spawning new instances; without this the
  // spent ones sit in `scenes` forever, pinning their shaders/fonts. Call it from
  // the incoming scene once it's current (e.g. its init) to reclaim the rest.
  // Index-based load_scene/next_scene users simply never call this, so their
  // stable-index assumptions are untouched.
  void drop_other_scenes() {
    scenes.erase(std::remove_if(scenes.begin(), scenes.end(),
                                [&](const std::shared_ptr<Scene> &s) {
                                  return s != current_scene;
                                }),
                 scenes.end());
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
