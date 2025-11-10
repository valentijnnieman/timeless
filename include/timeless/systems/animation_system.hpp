#pragma once
#include "timeless/systems/system.hpp"

class AnimationSystem : public System {
public:
  AnimationSystem(){};

  void update(ComponentManager &cm, float delta_time) {
    for (auto entity : registered_entities) {
      auto animation = cm.get_component<Animation>(entity);
      animation->update(delta_time); // Assuming a fixed timestep for simplicity
    }
  }
};
