#pragma once
#include "timeless/systems/system.hpp"

class CameraSystem : public System {
public:
  CameraSystem(){};

  void update(ComponentManager &cm, float delta_time) {
    for (auto entity : registered_entities) {
      auto camera = cm.get_component<Camera>(entity);
      camera->update(delta_time); // Assuming a fixed timestep for simplicity
      camera->update_orbit(delta_time); // Assuming a fixed timestep for simplicity
    }
  }
};
