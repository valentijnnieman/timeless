#pragma once
#include <memory>
#include <vector>
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

private:
  // A listener in the order it hears an event. Weak, so one removed by an
  // earlier handler of the same event is skipped.
  template <typename T> struct Ordered {
    int layer;
    Entity entity;
    std::weak_ptr<MouseInputListener<T>> listener;
  };

  // The listeners of `entities`, in the order they hear an event: by layer,
  // highest first, keeping their order within a layer.
  template <typename T>
  std::vector<Ordered<T>> in_layer_order(ComponentManager &cm,
                                         const std::vector<Entity> &entities);
};
