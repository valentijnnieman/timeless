#pragma once
#include "timeless/event.hpp"
#include "timeless/systems/system.hpp"

class EventSystem : public System {
private:
  template <typename T>
  void notify_listener(ComponentManager &cm, Event *event, Entity entity,
                       T *data) {
    auto listener = cm.get_component<EventListener<Event>>(entity);
    if (listener != nullptr) {
      listener->callback_handler<T>(event, entity, data);
    }
  }

  template <typename T>
  void notify_listener(ComponentManager &cm, PositionEvent *event,
                       Entity entity, T *data) {
    auto listener = cm.get_component<EventListener<PositionEvent>>(entity);
    if (listener != nullptr) {
      listener->callback_handler<T>(event, entity, data);
    }
  }

public:
  EventSystem(){};

  template <typename T>
  bool create_event(ComponentManager &cm, const std::string &event_type,
                    T *data) {
    Event *event = new Event(event_type);
    // Copy the list so that handlers which remove entities (via
    // TE::remove_entity) — e.g. a block finalizing and tearing itself down — do
    // not invalidate the iterator. notify_listener already null-checks the
    // component, so removed entities are silently skipped.
    auto entities = registered_entities;
    for (const auto &entity : entities) {
      notify_listener<T>(cm, event, entity, data);
    }
    if (event->picked_up) {
      delete event;
      return true;
    }
    delete event;
    return false;
  }

  template <typename T>
  bool create_position_event(ComponentManager &cm,
                             const std::string &event_type, glm::vec2 position,
                             T *data) {
    PositionEvent *event = new PositionEvent(event_type, position);
    // Copy first — the DropImage handler tears down the finished block (calling
    // TE::remove_entity on this very listener entity), which would otherwise
    // invalidate the iterator mid-loop. Skipped-because-removed entities are
    // handled by notify_listener's null check.
    auto entities = registered_entities;
    for (const auto &entity : entities) {
      notify_listener<T>(cm, event, entity, data);
    }
    if (event->picked_up) {
      delete event;
      return true;
    }
    delete event;
    return false;
  }
};
