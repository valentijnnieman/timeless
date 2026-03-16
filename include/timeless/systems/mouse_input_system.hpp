#pragma once
#include "timeless/components/mouse_input_listener.hpp"
#include "timeless/event.hpp"
#include "timeless/systems/system.hpp"

class MouseInputSystem : public System {
public:
  Entity camera;

  void register_camera(Entity c);
  std::vector<Entity> registered_move_entities;

  void register_move_entity(Entity entity);
  void remove_move_entity(Entity entity);
  void sort(ComponentManager &cm);
  void notify_listener(ComponentManager &cm, MouseEvent *event, Entity entity);
  void notify_listener(ComponentManager &cm, MouseMoveEvent *event, Entity entity);
  void mouse_click_handler(ComponentManager &cm, MouseEvent *event);
  void mouse_release_handler(ComponentManager &cm, MouseEvent *event);
  void mouse_move_handler(ComponentManager &cm, MouseMoveEvent *event);
  void mouse_scroll_handler(ComponentManager &cm, MouseEvent *event);
  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
};
