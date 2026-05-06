#include "timeless/systems/mouse_input_system.hpp"
#include <algorithm>

void MouseInputSystem::register_camera(Entity c) { camera = c; }

void MouseInputSystem::register_move_entity(Entity entity) {
    registered_move_entities.push_back(entity);
}

void MouseInputSystem::remove_move_entity(Entity entity) {
    if (!registered_move_entities.empty()) {
        auto found = std::find_if(registered_move_entities.begin(),
                                  registered_move_entities.end(),
                                  [&](auto &e) { return e == entity; });
        if (found != registered_move_entities.end())
            registered_move_entities.erase(found);
    }
}

void MouseInputSystem::sort(ComponentManager &cm) {
    std::stable_sort(registered_entities.begin(), registered_entities.end(),
                     [&](const Entity &a, const Entity &b) {
                         auto trans_a = cm.get_component<Transform>(a);
                         auto trans_b = cm.get_component<Transform>(b);
                         if (trans_a == nullptr || trans_b == nullptr)
                             return false;
                         return trans_a->position.z > trans_b->position.z;
                     });
}

void MouseInputSystem::notify_listener(ComponentManager &cm, MouseEvent *event,
                                        Entity entity) {
    auto listener = cm.get_component<MouseInputListener<MouseEvent>>(entity);
    if (listener != nullptr)
        listener->on_click_handler(event, entity, 0);
}

void MouseInputSystem::notify_listener(ComponentManager &cm,
                                        MouseMoveEvent *event, Entity entity) {
    auto listener = cm.get_component<MouseInputListener<MouseMoveEvent>>(entity);
    if (listener != nullptr)
        listener->on_click_handler(event, entity, 0);
}

void MouseInputSystem::mouse_click_handler(ComponentManager &cm,
                                            MouseEvent *event) {
    auto cam = cm.get_component<Camera>(camera);

    std::string et = event->eventType;
    event->eventType = "Global" + et;

    // Copy the list so that handlers which remove entities (via TE::remove_entity)
    // don't invalidate the iterator.  notify_listener already null-checks the
    // component, so removed entities are silently skipped.
    auto entities = registered_entities;
    for (const auto &entity : entities)
        notify_listener(cm, event, entity);
    delete event;
}

void MouseInputSystem::mouse_release_handler(ComponentManager &cm,
                                              MouseEvent *event) {
    auto entities = registered_entities;
    for (const auto &entity : entities)
        notify_listener(cm, event, entity);
    delete event;
}

void MouseInputSystem::mouse_move_handler(ComponentManager &cm,
                                           MouseMoveEvent *event) {
    for (const auto &entity : this->registered_move_entities) {
        notify_listener(cm, event, entity);
        if (event->picked_up)
            break;
    }
    delete event;
}

void MouseInputSystem::mouse_scroll_handler(ComponentManager &cm,
                                             MouseEvent *event) {
    auto entities = registered_entities;
    for (const auto &entity : entities) {
        notify_listener(cm, event, entity);
        if (event->picked_up)
            break;
    }
    delete event;
}

void MouseInputSystem::scroll_callback(GLFWwindow *window, double xoffset,
                                        double yoffset) {
    // ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X *
    // yoffset) * 0.5f; ViewportSettings::SCR_VIEWPORT_Y +=
    // (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
}
