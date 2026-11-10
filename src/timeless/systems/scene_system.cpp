#include "timeless/systems/scene_system.hpp"
#include "timeless/settings.hpp"
#include <algorithm>

void SceneSystem::switch_to(std::shared_ptr<Scene> scene) {
  auto it = std::find(scenes.begin(), scenes.end(), scene);
  if (it == scenes.end()) {
    scenes.push_back(scene);
    it = scenes.end() - 1;
  }
  transition_pending = true;
  pending_index = static_cast<int>(it - scenes.begin());
}

void SceneSystem::restart(std::shared_ptr<Scene> fresh) {
  auto it = std::find(scenes.begin(), scenes.end(), current_scene);
  if (it == scenes.end()) return;
  *it = fresh;
  transition_pending = true;
  pending_index = static_cast<int>(it - scenes.begin());
}

void SceneSystem::drop_other_scenes() {
  scenes.erase(std::remove_if(scenes.begin(), scenes.end(),
                              [&](const std::shared_ptr<Scene> &s) {
                                return s != current_scene;
                              }),
               scenes.end());
}

void SceneSystem::apply_pending() {
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
