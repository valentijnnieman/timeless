#pragma once
#include "timeless/systems/system.hpp"

class CameraSystem : public System {
public:
  CameraSystem() {}

  void update(ComponentManager &cm, float delta_time);
};
