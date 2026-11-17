#pragma once
#include "timeless/scene.hpp"
#include "timeless/systems/system.hpp"
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Owns the set of scenes and performs (deferred) transitions between them.
// Engine-level: it only ever touches the generic `Scene` interface.
//
// SceneSystem is the scenes' only owner. Everyone else reaches the current one
// through get_current_scene(), a non-owning pointer: use it, don't keep it,
// since restart() replaces (and destroys) the scene it points to.
class SceneSystem : public System {
private:
  struct Entry {
    std::string name; // for find_scene(); may be empty
    std::unique_ptr<Scene> scene;
  };

  std::vector<Entry> scenes;
  std::size_t current = 0;
  // The scene to switch to at the next apply_pending(); empty if none.
  std::optional<std::size_t> pending;
  // restart()'s replacement for the current scene. Swapped in by
  // apply_pending(), not at once: the scene it replaces may be the one
  // calling restart(), from its own update() or a callback.
  std::unique_ptr<Scene> replacement;

public:
  // Appends `scene` (next_scene() walks them in the order they're added),
  // optionally under a `name` for find_scene(), and returns its index.
  std::size_t add_scene(std::unique_ptr<Scene> scene, std::string name = {}) {
    scenes.push_back({std::move(name), std::move(scene)});
    return scenes.size() - 1;
  }

  // The index of the scene added under `name`; empty if there's none.
  std::optional<std::size_t> find_scene(const std::string &name) const;

  // The names given to add_scene(), comma-separated, for messages.
  std::string scene_names() const;

  // Start at the scene at `index`, without remove()'ing the one before: for
  // choosing the first scene, before the main loop runs.
  void set_current_scene(std::size_t index);

  // The current scene, or nullptr if there are none.
  Scene *get_current_scene() const {
    return current < scenes.size() ? scenes[current].scene.get() : nullptr;
  }

  std::size_t current_index() const { return current; }
  std::size_t scene_count() const { return scenes.size(); }

  // Safe to call from event callbacks — defers the actual switch to
  // apply_pending(). The last request of a frame wins.
  void next_scene() { load_scene(current + 1); }

  void load_scene(std::size_t index) {
    pending = index;
    replacement.reset();
  }

  // Restart the current scene as `fresh`, a newly built copy of it, in its
  // place, so next_scene() carries on as before. Deferred to apply_pending()
  // too, which remove()s and then destroys the old one.
  void restart(std::unique_ptr<Scene> fresh);

  // Call once per frame at the top of the game loop, before any scene logic.
  void apply_pending();
};
