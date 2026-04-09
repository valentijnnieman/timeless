#pragma once
#include "timeless/systems/system.hpp"
#include "timeless/components/node_animation.hpp"

class AnimationSystem : public System {
public:
  AnimationSystem() {}

  void update(ComponentManager &cm, float delta_time);
};
