#pragma once
#include "../components/component.hpp"
#include "../components/animation.hpp"
#include "../components/behaviour.hpp"
#include "../components/camera.hpp"
#include "../components/collider.hpp"
#include "../components/event_listener.hpp"
#include "../components/font.hpp"
#include "../components/keyboard_input_listener.hpp"
#include "../components/line.hpp"
#include "../components/mouse_input_listener.hpp"
#include "../components/movement_controller.hpp"
#include "../components/node.hpp"
#include "../components/shader.hpp"
#include "../components/sprite.hpp"
#include "../components/text.hpp"
#include "../components/texture.hpp"
#include "../components/transform.hpp"
#include "../entity.hpp"
#include "../components/quad.hpp"
#include "timeless/event.hpp"
#include <memory>
#include <unordered_map>

class ComponentManager {
public:
  std::unordered_map<Entity, std::shared_ptr<Quad>> quads;
  std::unordered_map<Entity, std::shared_ptr<Texture>> textures;
  std::unordered_map<Entity, std::shared_ptr<Shader>> shaders;
  std::unordered_map<Entity, std::shared_ptr<Sprite>> sprites;
  std::unordered_map<Entity, std::shared_ptr<Transform>> transforms;
  std::unordered_map<Entity, std::shared_ptr<MouseInputListener<MouseEvent>>>
      mouse_input_listeners;
  std::unordered_map<Entity,
                     std::shared_ptr<MouseInputListener<MouseMoveEvent>>>
      mouse_move_listeners;
  std::unordered_map<Entity, std::shared_ptr<EventListener<Event>>>
      event_listeners;
  std::unordered_map<Entity, std::shared_ptr<EventListener<PositionEvent>>>
      position_event_listeners;
  std::unordered_map<Entity, std::shared_ptr<KeyboardInputListener>>
      keyboard_input_listeners;
  std::unordered_map<Entity, std::shared_ptr<Camera>> cameras;
  std::unordered_map<Entity, std::shared_ptr<Collider>> colliders;
  std::unordered_map<Entity, std::shared_ptr<MovementController>> movements;
  std::unordered_map<Entity, std::shared_ptr<Node>> nodes;
  std::unordered_map<Entity, std::shared_ptr<Font>> fonts;
  std::unordered_map<Entity, std::shared_ptr<Text>> texts;
  std::unordered_map<Entity, std::shared_ptr<Behaviour>> behaviours;
  std::unordered_map<Entity, std::shared_ptr<Line>> geometry;
  std::unordered_map<Entity, std::shared_ptr<Animation>> animations;

  ComponentManager(){};

  void add_component(Entity entity, Quad *quad) {
    quads.insert({entity, std::shared_ptr<Quad>(quad)});
  }
  void add_component(Entity entity, std::shared_ptr<Quad> quad) {
    quads.insert({entity, quad});
  }
  void add_component(Entity entity, Texture *texture) {
    textures.insert({entity, std::shared_ptr<Texture>(texture)});
  }
  void add_component(Entity entity, std::shared_ptr<Texture> texture) {
    textures.insert({entity, texture});
  }
  void add_component(Entity entity, Shader *shader) {
    shaders.insert({entity, std::shared_ptr<Shader>(shader)});
  }
  void add_component(Entity entity, std::shared_ptr<Shader> shader) {
    shaders.insert({entity, shader});
  }
  void add_component(Entity entity, Sprite *sprite) {
    sprites.insert({entity, std::shared_ptr<Sprite>(sprite)});
  }
  void add_component(Entity entity, Transform *transform) {
    transforms.insert({entity, std::shared_ptr<Transform>(transform)});
  }
  void add_component(Entity entity,
                     MouseInputListener<MouseEvent> *mouse_input) {
    mouse_input_listeners.insert(
        {entity, std::shared_ptr<MouseInputListener<MouseEvent>>(mouse_input)});
  }
  void add_component(Entity entity,
                     MouseInputListener<MouseMoveEvent> *mouse_input) {
    mouse_move_listeners.insert(
        {entity,
         std::shared_ptr<MouseInputListener<MouseMoveEvent>>(mouse_input)});
  }
  void add_component(Entity entity, EventListener<Event> *event_listener) {
    event_listeners.insert(
        {entity, std::shared_ptr<EventListener<Event>>(event_listener)});
  }
  void add_component(Entity entity,
                     EventListener<PositionEvent> *event_listener) {
    position_event_listeners.insert(
        {entity,
         std::shared_ptr<EventListener<PositionEvent>>(event_listener)});
  }
  void add_component(Entity entity, KeyboardInputListener *keyboard_input) {
    keyboard_input_listeners.insert(
        {entity, std::shared_ptr<KeyboardInputListener>(keyboard_input)});
  }
  void add_component(Entity entity, Camera *camera) {
    cameras.insert({entity, std::shared_ptr<Camera>(camera)});
  }
  void add_component(Entity entity, std::shared_ptr<Camera> camera) {
    cameras.insert({entity, camera});
  }
  void add_component(Entity entity, MovementController *movement) {
    movements.insert({entity, std::shared_ptr<MovementController>(movement)});
  }
  void add_component(Entity entity, Collider *collider) {
    colliders.insert({entity, std::shared_ptr<Collider>(collider)});
  }
  void add_component(Entity entity, Node *node) {
    nodes.insert({entity, std::shared_ptr<Node>(node)});
  }
  void add_component(Entity entity, std::shared_ptr<Node> node) {
    nodes.insert({entity, node});
  }
  void add_component(Entity entity, Font *font) {
    fonts.insert({entity, std::shared_ptr<Font>(font)});
  }
  void add_component(Entity entity, std::shared_ptr<Font> font) {
    fonts.insert({entity, font});
  }
  void add_component(Entity entity, Text *text) {
    texts.insert({entity, std::shared_ptr<Text>(text)});
  }
  void add_component(Entity entity, Behaviour *behaviour) {
    behaviours.insert({entity, std::shared_ptr<Behaviour>(behaviour)});
  }
  void add_component(Entity entity, Line *line) {
    geometry.insert({entity, std::shared_ptr<Line>(line)});
  }
  void add_component(Entity entity, Animation *animation) {
    animations.insert({entity, std::shared_ptr<Animation>(animation)});
  }

  template <class T> const inline std::shared_ptr<T> get_component(Entity entity) {
    return nullptr;
  }
  /** this method completely removes all components for an entity */
  void remove_entity(Entity entity) {
    try {
      mouse_input_listeners.erase(entity);
      keyboard_input_listeners.erase(entity);
      animations.erase(entity);
      quads.erase(entity);
      textures.erase(entity);
      shaders.erase(entity);
      sprites.erase(entity);
      transforms.erase(entity);
      geometry.erase(entity);
      cameras.erase(entity);
      colliders.erase(entity);
      movements.erase(entity);
      nodes.erase(entity);
      fonts.erase(entity);
      texts.erase(entity);
      behaviours.erase(entity);
      event_listeners.erase(entity);
      position_event_listeners.erase(entity);
    } catch (std::exception e) {
      throw e;
    }
  }
};

template <>
const inline std::shared_ptr<Node> ComponentManager::get_component<Node>(Entity entity) {
  if (nodes.contains(entity))
    return nodes.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Quad> ComponentManager::get_component<Quad>(Entity entity) {
  if (quads.contains(entity))
    return quads.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Texture>
ComponentManager::get_component<Texture>(Entity entity) {
  if (textures.contains(entity))
    return textures.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Shader> ComponentManager::get_component<Shader>(Entity entity) {
  if (shaders.contains(entity))
    return shaders.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Transform>
ComponentManager::get_component<Transform>(Entity entity) {
  if (transforms.contains(entity))
    return transforms.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Sprite> ComponentManager::get_component<Sprite>(Entity entity) {
  if (sprites.contains(entity))
    return sprites.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Camera> ComponentManager::get_component<Camera>(Entity entity) {
  if (cameras.contains(entity))
    return cameras.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Font> ComponentManager::get_component<Font>(Entity entity) {
  if (fonts.contains(entity))
    return fonts.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Behaviour>
ComponentManager::get_component<Behaviour>(Entity entity) {
  if (behaviours.contains(entity))
    return behaviours.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Text> ComponentManager::get_component<Text>(Entity entity) {
  if (texts.contains(entity))
    return texts.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<MouseInputListener<MouseEvent>>
ComponentManager::get_component<MouseInputListener<MouseEvent>>(Entity entity) {
  if (mouse_input_listeners.contains(entity))
    return mouse_input_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<MouseInputListener<MouseMoveEvent>>
ComponentManager::get_component<MouseInputListener<MouseMoveEvent>>(
    Entity entity) {
  if (mouse_move_listeners.contains(entity))
    return mouse_move_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<EventListener<Event>>
ComponentManager::get_component<EventListener<Event>>(Entity entity) {
  if (event_listeners.contains(entity))
    return event_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<EventListener<PositionEvent>>
ComponentManager::get_component<EventListener<PositionEvent>>(Entity entity) {
  if (position_event_listeners.contains(entity))
    return position_event_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<KeyboardInputListener>
ComponentManager::get_component<KeyboardInputListener>(Entity entity) {
  if (keyboard_input_listeners.contains(entity))
    return keyboard_input_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Animation>
ComponentManager::get_component<Animation>(Entity entity) {
  if (animations.contains(entity))
    return animations.at(entity);
  return nullptr;
}
