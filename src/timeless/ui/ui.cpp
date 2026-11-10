#include "timeless/ui/ui.hpp"
#include "timeless/components/event_listener.hpp"
#include "timeless/components/sprite.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/settings.hpp"
#include "timeless/systems/event_system.hpp"

void UIComponent::update_position() {
  glm::vec2 resolution = glm::vec2(TESettings::SCREEN_X, TESettings::SCREEN_Y);
  position.x = resolution.x * anchor.x + margin.x;
  position.y = resolution.y * anchor.y + margin.y;

  auto transform = TE::get_component<Transform>(bg_ent);
  if(transform != nullptr) {
    transform->set_position(position);
  }

  auto main_transform = TE::get_component<Transform>(main_ent);
  if(main_transform != nullptr) {
    main_transform->set_position(position);
  }
}

void UIComponent::create_animation() {
  for (auto entity : entities) {
    auto transform = TE::get_component<Transform>(entity);
    glm::vec3 og_position = transform->position;
    TE::get_component<Transform>(entity)->set_position_frames(
        glm::vec3(og_position.x - 100.0, og_position.y, 0.0), og_position,
        20.0);
  }
}

void UIComponent::hover_animation() {
  auto transform = TE::get_component<Transform>(bg_ent);

  if(transform != nullptr) {
    glm::vec3 og_scale = glm::vec3(1.0);
    transform->set_scale_frames(og_scale,
    glm::vec3(1.25), 10.0);
			TE::get_component<Transform>(bg_ent)->reset = false;
  }
}

void UIComponent::exit_animation() {
  auto transform = TE::get_component<Transform>(bg_ent);

  if(transform != nullptr) {
    glm::vec3 og_scale = glm::vec3(1.0);
    transform->append_scale_frames(glm::vec3(1.25), og_scale, 10.0);
    transform->scale = glm::vec3(1.0);
  }
}

void UIComponent::remove() {
  if (removed) return;
  removed = true;
  for (auto entity : entities) {
    TE::remove_entity(entity);
  }
}

void UIComponent::add_close_event_listener(Entity entity) {
  TE::add_component<EventListener<Event>>(
      entity, new EventListener<Event>([&](Event *event, Entity entity,
                                           void *data) mutable {
        if (event->eventType == "CloseUI") {
          remove();
        }
      }));
  TE::get_system<EventSystem>("EventSystem")->register_entity(entity);
}

void UIComponent::add_hover_event_listener() {
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
