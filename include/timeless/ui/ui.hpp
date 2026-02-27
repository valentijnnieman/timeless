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

  glm::vec3 position;
  glm::vec2 anchor;
  glm::vec2 margin;

  UIComponent(glm::vec2 anchor, glm::vec2 margin) : anchor(anchor), margin(margin) {
    main_ent = create_entity("uicomponent_main"); 
    bg_ent = create_entity("uicomponent_bg");

    add_entity(main_ent);
    add_entity(bg_ent);
  }

  UIComponent() {
    main_ent = create_entity("uicomponent_main"); 
    bg_ent = create_entity("uicomponent_bg");

    add_entity(main_ent);
    add_entity(bg_ent);
  }

  void update_position() {
    glm::vec2 resolution = glm::vec2(TESettings::SCREEN_X, TESettings::SCREEN_Y);
    position.x = resolution.x * anchor.x + margin.x;
    position.y = resolution.y * anchor.y + margin.y;

    auto transform = TE::get_component<Transform>(bg_ent);
    if(transform != nullptr) {
      transform->set_position(position);
    }
  }

  void add_entity(Entity entity) { 
    entities.push_back(entity); 
  }

  virtual void create_animation() {
    for (auto entity : entities) {
      auto transform = TE::get_component<Transform>(entity);
      glm::vec3 og_position = transform->position;
      TE::get_component<Transform>(entity)->set_position_frames(
          glm::vec3(og_position.x - 100.0, og_position.y, 0.0), og_position,
          20.0);
    }
  }

  virtual void hover_animation() {
    auto transform = TE::get_component<Transform>(bg_ent);

    if(transform != nullptr) {
      glm::vec3 og_scale = glm::vec3(1.0);
      transform->set_scale_frames(og_scale,
      glm::vec3(1.25), 10.0);
			TE::get_component<Transform>(bg_ent)->reset = false;
    }
  }

  virtual void exit_animation() {
    auto transform = TE::get_component<Transform>(bg_ent);

    if(transform != nullptr) {
      glm::vec3 og_scale = glm::vec3(1.0);
      transform->append_scale_frames(glm::vec3(1.25), og_scale, 10.0);
      transform->scale = glm::vec3(1.0);
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

  virtual void add_hover_event_listener() {
    TE::add_component<MouseInputListener<MouseMoveEvent>>(
        bg_ent,
        new MouseInputListener<MouseMoveEvent>(
            [&](
                MouseMoveEvent *event, Entity entity, int data) mutable {
              if (event->eventType == "MouseMove" &&
                  TE::hovered_over(event, entity)) {
                auto sprite = TE::get_component<Sprite>(entity);
                if (sprite != nullptr) {
                  if(!sprite->hidden) {
                    // event->picked_up = true;
                    if(!hovering) {
                      hovering = true;
                      hover_animation();
                    }
                  } else {
                    // event->picked_up = true;
                    hovering = false;
                  }
                }
              } else {
                // event->picked_up = true;
                if(hovering) {
                  exit_animation();
                }
                hovering = false;
              }
            }));
  }

};
