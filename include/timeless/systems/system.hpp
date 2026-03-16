#pragma once
#include "timeless/entity.hpp"
#include "timeless/managers/component_manager.hpp"

class System {
public:
  std::vector<Entity> registered_entities;

  virtual void register_entity(Entity entity);
  virtual void remove_entity(Entity entity);
  virtual void clear(ComponentManager &cm);
  virtual int get_registered_entity_count();
  virtual void purge();
  virtual void update(ComponentManager &cm, GLFWwindow *window) {}
  virtual void update(ComponentManager &cm) {}
  virtual void render(ComponentManager &cm, int x, int y, float zoom = 1.0,
                      int tick = 0) {}
  virtual void register_camera(Entity entity) {}
};
