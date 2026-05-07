#pragma once
#include "timeless/components/component.hpp"
#include "timeless/components/animation.hpp"
#include "timeless/components/skeletal_animation.hpp"
#include "timeless/components/node_animation.hpp"
#include "timeless/components/behaviour.hpp"
#include "timeless/components/camera.hpp"
#include "timeless/components/collider.hpp"
#include "timeless/components/event_listener.hpp"
#include "timeless/components/font.hpp"
#include "timeless/components/keyboard_input_listener.hpp"
#include "timeless/components/line.hpp"
#include "timeless/components/mouse_input_listener.hpp"
#include "timeless/components/movement_controller.hpp"
#include "timeless/components/node.hpp"
#include "timeless/components/particle.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/sprite.hpp"
#include "timeless/components/text.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/entity.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/model.hpp"
#include "timeless/event.hpp"
#include "timeless/mutex.hpp"
#include <memory>

#include <unordered_map>
#include <map>

class ComponentManager {
private:
 mutable SharedMutex manager_mutex; // 'mutable' allows locking in const functions
public:
  std::unordered_map<Entity, std::shared_ptr<Quad>> quads;
  std::unordered_map<Entity, std::shared_ptr<Model>> models;
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
  std::map<Entity, std::shared_ptr<Node>> nodes;
  std::unordered_map<Entity, std::shared_ptr<Font>> fonts;
  std::unordered_map<Entity, std::shared_ptr<Text>> texts;
  std::unordered_map<Entity, std::shared_ptr<Behaviour>> behaviours;
  std::unordered_map<Entity, std::shared_ptr<Line>> geometry;
  std::unordered_map<Entity, std::shared_ptr<Animation>> animations;
  std::unordered_map<Entity, std::shared_ptr<SkeletalAnimation>> skeletal_animations;
  std::unordered_map<Entity, std::shared_ptr<NodeAnimation>> node_animations;
  std::unordered_map<Entity, std::shared_ptr<ParticleEmitter>> particleemitters;

  ComponentManager(){};

  void add_component(Entity entity, Quad *quad) {
    std::unique_lock lock(manager_mutex);
    quads.insert_or_assign(entity, std::shared_ptr<Quad>(quad));
  }
  void add_component(Entity entity, std::shared_ptr<Quad> quad) {
    std::unique_lock lock(manager_mutex);
    quads.insert_or_assign(entity, quad);
  }
  void add_component(Entity entity, Model *model) {
    std::unique_lock lock(manager_mutex);
    models.insert_or_assign(entity, std::shared_ptr<Model>(model));
  }
  void add_component(Entity entity, std::shared_ptr<Model> model) {
    std::unique_lock lock(manager_mutex);
    models.insert_or_assign(entity, model);
  }
  void add_component(Entity entity, Texture *texture) {
    std::unique_lock lock(manager_mutex);
    textures.insert_or_assign(entity, std::shared_ptr<Texture>(texture));
  }
  void add_component(Entity entity, std::shared_ptr<Texture> texture) {
    std::unique_lock lock(manager_mutex);
    textures.insert_or_assign(entity, texture);
  }
  void add_component(Entity entity, Shader *shader) {
    std::unique_lock lock(manager_mutex);
    shaders.insert_or_assign(entity, std::shared_ptr<Shader>(shader));
  }
  void add_component(Entity entity, std::shared_ptr<Shader> shader) {
    std::unique_lock lock(manager_mutex);
    shaders.insert_or_assign(entity, shader);
  }
  void add_component(Entity entity, Sprite *sprite) {
    std::unique_lock lock(manager_mutex);
    sprites.insert_or_assign(entity, std::shared_ptr<Sprite>(sprite));
  }
  void add_component(Entity entity, Transform *transform) {
    std::unique_lock lock(manager_mutex);
    transforms.insert_or_assign(entity, std::shared_ptr<Transform>(transform));
  }
  void add_component(Entity entity,
                     MouseInputListener<MouseEvent> *mouse_input) {
    std::unique_lock lock(manager_mutex);
    mouse_input_listeners.insert_or_assign(
        entity, std::shared_ptr<MouseInputListener<MouseEvent>>(mouse_input));
  }
  void add_component(Entity entity,
                     MouseInputListener<MouseMoveEvent> *mouse_input) {
    std::unique_lock lock(manager_mutex);
    mouse_move_listeners.insert_or_assign(
        entity, std::shared_ptr<MouseInputListener<MouseMoveEvent>>(mouse_input));
  }
  void add_component(Entity entity, EventListener<Event> *event_listener) {
    std::unique_lock lock(manager_mutex);
    event_listeners.insert_or_assign(
        entity, std::shared_ptr<EventListener<Event>>(event_listener));
  }
  void add_component(Entity entity,
                     EventListener<PositionEvent> *event_listener) {
    std::unique_lock lock(manager_mutex);
    position_event_listeners.insert_or_assign(
        entity, std::shared_ptr<EventListener<PositionEvent>>(event_listener));
  }
  void add_component(Entity entity, KeyboardInputListener *keyboard_input) {
    std::unique_lock lock(manager_mutex);
    keyboard_input_listeners.insert_or_assign(
        entity, std::shared_ptr<KeyboardInputListener>(keyboard_input));
  }
  void add_component(Entity entity, Camera *camera) {
    std::unique_lock lock(manager_mutex);
    cameras.insert_or_assign(entity, std::shared_ptr<Camera>(camera));
  }
  void add_component(Entity entity, std::shared_ptr<Camera> camera) {
    std::unique_lock lock(manager_mutex);
    cameras.insert_or_assign(entity, camera);
  }
  void add_component(Entity entity, MovementController *movement) {
    std::unique_lock lock(manager_mutex);
    movements.insert_or_assign(entity, std::shared_ptr<MovementController>(movement));
  }
  void add_component(Entity entity, Collider *collider) {
    std::unique_lock lock(manager_mutex);
    colliders.insert_or_assign(entity, std::shared_ptr<Collider>(collider));
  }
  void add_component(Entity entity, Node *node) {
    std::unique_lock lock(manager_mutex);
    nodes.insert_or_assign(entity, std::shared_ptr<Node>(node));
  }
  void add_component(Entity entity, std::shared_ptr<Node> node) {
    std::unique_lock lock(manager_mutex);
    nodes.insert_or_assign(entity, node);
  }
  void add_component(Entity entity, Font *font) {
    std::unique_lock lock(manager_mutex);
    fonts.insert_or_assign(entity, std::shared_ptr<Font>(font));
  }
  void add_component(Entity entity, std::shared_ptr<Font> font) {
    std::unique_lock lock(manager_mutex);
    fonts.insert_or_assign(entity, font);
  }
  void add_component(Entity entity, Text *text) {
    std::unique_lock lock(manager_mutex);
    texts.insert_or_assign(entity, std::shared_ptr<Text>(text));
  }
  void add_component(Entity entity, Behaviour *behaviour) {
    std::unique_lock lock(manager_mutex);
    behaviours.insert_or_assign(entity, std::shared_ptr<Behaviour>(behaviour));
  }
  void add_component(Entity entity, Line *line) {
    std::unique_lock lock(manager_mutex);
    geometry.insert_or_assign(entity, std::shared_ptr<Line>(line));
  }
  void add_component(Entity entity, Animation *animation) {
    std::unique_lock lock(manager_mutex);
    animations.insert_or_assign(entity, std::shared_ptr<Animation>(animation));
  }
  void add_component(Entity entity, std::shared_ptr<Animation> animation) {
    std::unique_lock lock(manager_mutex);
    animations.insert_or_assign(entity, animation);
  }
  void add_component(Entity entity, SkeletalAnimation *animation) {
    std::unique_lock lock(manager_mutex);
    skeletal_animations.insert_or_assign(entity, std::shared_ptr<SkeletalAnimation>(animation));
  }
  void add_component(Entity entity, std::shared_ptr<SkeletalAnimation> animation) {
    std::unique_lock lock(manager_mutex);
    skeletal_animations.insert_or_assign(entity, animation);
  }
  void add_component(Entity entity, NodeAnimation *animation) {
    std::unique_lock lock(manager_mutex);
    node_animations.insert_or_assign(entity, std::shared_ptr<NodeAnimation>(animation));
  }
  void add_component(Entity entity, std::shared_ptr<NodeAnimation> animation) {
    std::unique_lock lock(manager_mutex);
    node_animations.insert_or_assign(entity, animation);
  }
  void add_component(Entity entity, ParticleEmitter *pe) {
    std::unique_lock lock(manager_mutex);
    particleemitters.insert_or_assign(entity, std::shared_ptr<ParticleEmitter>(pe));
  }
  void add_component(Entity entity, std::shared_ptr<ParticleEmitter> pe) {
    std::unique_lock lock(manager_mutex);
    particleemitters.insert_or_assign(entity, pe);
  }

  template <class T> const inline std::shared_ptr<T> get_component(Entity entity) {
    return nullptr;
  }

  template <class T> const inline void remove_component(Entity entity) {
  }
  void does_component_exist(Entity entity) {
    if (
        quads.contains(entity) ||
        models.contains(entity) ||
        textures.contains(entity) ||
        shaders.contains(entity) ||
        sprites.contains(entity) ||
        transforms.contains(entity) ||
        mouse_input_listeners.contains(entity) ||
        mouse_move_listeners.contains(entity) ||
        event_listeners.contains(entity) ||
        position_event_listeners.contains(entity) ||
        keyboard_input_listeners.contains(entity) ||
        cameras.contains(entity) ||
        colliders.contains(entity) ||
        movements.contains(entity) ||
        nodes.contains(entity) ||
        fonts.contains(entity) ||
        texts.contains(entity) ||
        behaviours.contains(entity) ||
        geometry.contains(entity) ||
        animations.contains(entity)
    ) {
        // Entity exists in at least one map
        std::cout << "Entity " << entity << " already has components!" << std::endl;
    }
  }
  /** this method completely removes all components for an entity */
  void remove_entity(Entity entity, bool destroy = true) {
    std::unique_lock lock(manager_mutex);
    try {
      quads.erase(entity);
      models.erase(entity);
      textures.erase(entity);
      shaders.erase(entity);
      sprites.erase(entity);
      transforms.erase(entity);
      mouse_input_listeners.erase(entity);
      mouse_move_listeners.erase(entity);
      event_listeners.erase(entity);
      position_event_listeners.erase(entity);
      keyboard_input_listeners.erase(entity);
      cameras.erase(entity);
      colliders.erase(entity);
      movements.erase(entity);
      nodes.erase(entity);
      fonts.erase(entity);
      texts.erase(entity);
      behaviours.erase(entity);
      geometry.erase(entity);
      animations.erase(entity);
      skeletal_animations.erase(entity);
      node_animations.erase(entity);
      particleemitters.erase(entity);
      if(destroy) {
        destroy_entity(entity);
      }
    } catch (std::exception e) {
      throw e;
    }
  }
  void cleanup() {
    std::unique_lock lock(manager_mutex);
    quads.clear();
    models.clear();
    textures.clear();
    shaders.clear();
    sprites.clear();
    transforms.clear();
    mouse_input_listeners.clear();
    mouse_move_listeners.clear();
    event_listeners.clear();
    position_event_listeners.clear();
    keyboard_input_listeners.clear();
    cameras.clear();
    colliders.clear();
    movements.clear();
    nodes.clear();
    fonts.clear();
    texts.clear();
    behaviours.clear();
    geometry.clear();
    animations.clear();
    skeletal_animations.clear();
    node_animations.clear();
    particleemitters.clear();
  }
};
template <>
const inline void ComponentManager::remove_component<Shader>(Entity entity) {
  std::unique_lock lock(manager_mutex);
  if (shaders.contains(entity))
    shaders.erase(entity);
}

template <>
const inline std::shared_ptr<Node> ComponentManager::get_component<Node>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (nodes.contains(entity))
    return nodes.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Quad> ComponentManager::get_component<Quad>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (quads.contains(entity))
    return quads.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Model> ComponentManager::get_component<Model>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (models.contains(entity))
    return models.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Texture>
ComponentManager::get_component<Texture>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (textures.contains(entity))
    return textures.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Shader> ComponentManager::get_component<Shader>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (shaders.contains(entity))
    return shaders.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Transform>
ComponentManager::get_component<Transform>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (transforms.contains(entity))
    return transforms.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Sprite> ComponentManager::get_component<Sprite>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (sprites.contains(entity))
    return sprites.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Camera> ComponentManager::get_component<Camera>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (cameras.contains(entity))
    return cameras.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Font> ComponentManager::get_component<Font>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (fonts.contains(entity))
    return fonts.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Behaviour>
ComponentManager::get_component<Behaviour>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (behaviours.contains(entity))
    return behaviours.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Text> ComponentManager::get_component<Text>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (texts.contains(entity))
    return texts.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<MouseInputListener<MouseEvent>>
ComponentManager::get_component<MouseInputListener<MouseEvent>>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (mouse_input_listeners.contains(entity))
    return mouse_input_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<MouseInputListener<MouseMoveEvent>>
ComponentManager::get_component<MouseInputListener<MouseMoveEvent>>(
    Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (mouse_move_listeners.contains(entity))
    return mouse_move_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<EventListener<Event>>
ComponentManager::get_component<EventListener<Event>>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (event_listeners.contains(entity))
    return event_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<EventListener<PositionEvent>>
ComponentManager::get_component<EventListener<PositionEvent>>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (position_event_listeners.contains(entity))
    return position_event_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<KeyboardInputListener>
ComponentManager::get_component<KeyboardInputListener>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (keyboard_input_listeners.contains(entity))
    return keyboard_input_listeners.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<Animation>
ComponentManager::get_component<Animation>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (animations.contains(entity))
    return animations.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<SkeletalAnimation>
ComponentManager::get_component<SkeletalAnimation>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (skeletal_animations.contains(entity))
    return skeletal_animations.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<NodeAnimation>
ComponentManager::get_component<NodeAnimation>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (node_animations.contains(entity))
    return node_animations.at(entity);
  return nullptr;
}
template <>
const inline std::shared_ptr<ParticleEmitter>
ComponentManager::get_component<ParticleEmitter>(Entity entity) {
  std::shared_lock lock(manager_mutex);
  if (particleemitters.contains(entity))
    return particleemitters.at(entity);
  return nullptr;
}
