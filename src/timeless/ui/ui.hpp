#pragma once
#include <vector>
#include "timeless/entity.hpp"
#include "timeless/timeless.hpp"

class UIComponent {
private:
  std::vector<Entity> entities;

public:
  Entity main_ent;
  Entity bg_ent;
  bool hovering = false;

  UIComponent() {
    main_ent = create_entity(); 
    bg_ent = create_entity(); 

    add_entity(main_ent);
    add_entity(bg_ent);

    add_hover_event_listener();
  }

  void add_entity(Entity entity) { 
    entities.push_back(entity); 
    TE::add_component<Animation>(entity, new Animation());
    TE::get_system<AnimationSystem>("AnimationSystem")
        ->register_entity(entity);
  }

  virtual void create_animation() {
    for (auto entity : entities) {
      auto transform = TE::get_component<Transform>(entity);
      glm::vec3 og_scale = transform->scale;
      glm::vec3 og_position = transform->position;
      TE::get_component<Animation>(entity)->set_position_frames(
          glm::vec3(og_position.x - 100.0, og_position.y, 0.0), og_position,
          20.0);
    }
  }

  virtual void hover_animation() {
    for (auto entity : {bg_ent}) {
      auto transform = TE::get_component<Transform>(entity);

      if(transform != nullptr) {
        glm::vec3 og_scale = transform->scale;
        auto anim = TE::get_component<Animation>(entity);
        if(anim != nullptr) {
          anim->append_scale_frames(og_scale,
          og_scale + glm::vec3(3.0), 20.0);
        }
      }
    }
  }

  virtual void exit_animation() {
    for (auto entity : {bg_ent}) {
      auto transform = TE::get_component<Transform>(entity);

      if(transform != nullptr) {
        glm::vec3 og_scale = transform->scale;
        auto anim = TE::get_component<Animation>(entity);
        if(anim != nullptr) {
          anim->append_scale_frames(og_scale,og_scale - glm::vec3(3.0), 20.0);
        }
      }
    }
  }

  virtual void remove() {
    for (auto entity : entities) {
      TE::remove_entity(entity);
    }
  }

  virtual void add_close_event_listener(Entity entity) {
    TE::add_component<EventListener<Event>>(
        entity, new EventListener<Event>([&](Event *event, Entity entity,
                                             void *data) mutable {
          if (event->eventType == "CloseUI") {
            remove();                        
          }
        }));
    TE::get_system<EventSystem>("EventSystem")->register_entity(entity);
  }

  void add_hover_event_listener() {
    TE::add_component<MouseInputListener<MouseMoveEvent>>(
        bg_ent,
        new MouseInputListener<MouseMoveEvent>(
            [&](
                MouseMoveEvent *event, Entity entity, int data) {
              if (event->eventType == "MouseMove" &&
                  TE::hovered_over(event, entity, TESettings::ZOOM)) {
                auto sprite = TE::get_component<Sprite>(entity);
                if (sprite != nullptr) {
                  if(!sprite->hidden) {
                    // event->picked_up = true;
                    if(!hovering) {
                      hovering = true;
                      this->hover_animation();
                    }
                  } else {
                    // event->picked_up = true;
                    hovering = false;
                  }
                }
              } else {
                // event->picked_up = true;
                if(hovering) {
                  this->exit_animation();
                }
                hovering = false;
              }
            }));
  }

};
