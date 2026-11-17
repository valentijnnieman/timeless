#include "timeless/systems/scene_system.hpp"
#include "timeless/core.hpp"
#include <iostream>

std::optional<std::size_t> SceneSystem::find_scene(const std::string &name) const {
  for (std::size_t i = 0; i < scenes.size(); ++i)
    if (!scenes[i].name.empty() && scenes[i].name == name)
      return i;
  return std::nullopt;
}

std::string SceneSystem::scene_names() const {
  std::string names;
  for (const auto &entry : scenes)
    if (!entry.name.empty())
      names += (names.empty() ? "" : ", ") + entry.name;
  return names;
}

void SceneSystem::set_current_scene(std::size_t index) {
  if (index >= scenes.size()) {
    std::cerr << "[TIMELESS] SceneSystem: no scene " << index << " (there are "
              << scenes.size() << ")" << std::endl;
    return;
  }
  current = index;
}

void SceneSystem::restart(std::unique_ptr<Scene> fresh) {
  if (fresh == nullptr) return;
  pending = current;
  replacement = std::move(fresh);
}

void SceneSystem::apply_pending() {
  if (!pending) return;
  const std::size_t target = *pending;
  pending.reset();

  // E.g. next_scene() on the last scene: stay put rather than read past the end.
  if (target >= scenes.size()) {
    std::cerr << "[TIMELESS] SceneSystem: no scene " << target << " (there are "
              << scenes.size() << "), staying on scene " << current << std::endl;
    replacement.reset();
    return;
  }

  TE::set_zoom(1.0f);
  scenes[current].scene->remove();
  // Only restart() sets a replacement, and always for the current scene
  // (load_scene() clears it), so target == current here. Assigning destroys
  // the old scene, now that its remove() is done.
  if (replacement != nullptr)
    scenes[current].scene = std::move(replacement);
  current = target;
}
