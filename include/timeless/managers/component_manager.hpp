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
  mutable SharedMutex manager_mutex;

  // Tag dispatch — one overload per component type, returns ref to its map
  auto& map_for(Quad*)                               { return quads; }
  auto& map_for(Model*)                              { return models; }
  auto& map_for(Texture*)                            { return textures; }
  auto& map_for(Shader*)                             { return shaders; }
  auto& map_for(Sprite*)                             { return sprites; }
  auto& map_for(Transform*)                          { return transforms; }
  auto& map_for(MouseInputListener<MouseEvent>*)     { return mouse_input_listeners; }
  auto& map_for(MouseInputListener<MouseMoveEvent>*) { return mouse_move_listeners; }
  auto& map_for(EventListener<Event>*)               { return event_listeners; }
  auto& map_for(EventListener<PositionEvent>*)       { return position_event_listeners; }
  auto& map_for(KeyboardInputListener*)              { return keyboard_input_listeners; }
  auto& map_for(Camera*)                             { return cameras; }
  auto& map_for(Collider*)                           { return colliders; }
  auto& map_for(MovementController*)                 { return movements; }
  auto& map_for(Node*)                               { return nodes; }
  auto& map_for(Font*)                               { return fonts; }
  auto& map_for(Text*)                               { return texts; }
  auto& map_for(Behaviour*)                          { return behaviours; }
  auto& map_for(Line*)                               { return geometry; }
  auto& map_for(Animation*)                          { return animations; }
  auto& map_for(SkeletalAnimation*)                  { return skeletal_animations; }
  auto& map_for(NodeAnimation*)                      { return node_animations; }
  auto& map_for(ParticleEmitter*)                    { return particleemitters; }

public:
  std::unordered_map<Entity, std::shared_ptr<Quad>> quads;
  std::unordered_map<Entity, std::shared_ptr<Model>> models;
  std::unordered_map<Entity, std::shared_ptr<Texture>> textures;
  std::unordered_map<Entity, std::shared_ptr<Shader>> shaders;
  std::unordered_map<Entity, std::shared_ptr<Sprite>> sprites;
  std::unordered_map<Entity, std::shared_ptr<Transform>> transforms;
  std::unordered_map<Entity, std::shared_ptr<MouseInputListener<MouseEvent>>>
      mouse_input_listeners;
  std::unordered_map<Entity, std::shared_ptr<MouseInputListener<MouseMoveEvent>>>
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

  template <class T>
  void add_component(Entity entity, std::shared_ptr<T> comp) {
    std::unique_lock lock(manager_mutex);
    map_for(static_cast<T*>(nullptr)).insert_or_assign(entity, comp);
  }

  template <class T>
  void add_component(Entity entity, T* comp) {
    std::unique_lock lock(manager_mutex);
    map_for(static_cast<T*>(nullptr)).insert_or_assign(entity, std::shared_ptr<T>(comp));
  }

  template <class T>
  const inline std::shared_ptr<T> get_component(Entity entity) {
    std::shared_lock lock(manager_mutex);
    auto& m = map_for(static_cast<T*>(nullptr));
    auto it = m.find(entity);
    return it != m.end() ? it->second : nullptr;
  }

  template <class T>
  const inline void remove_component(Entity entity) {}

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
  shaders.erase(entity);
}
