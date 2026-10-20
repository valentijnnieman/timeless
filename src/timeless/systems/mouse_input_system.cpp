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

template <typename T>
std::vector<Entity>
MouseInputSystem::in_layer_order(ComponentManager &cm,
                                 const std::vector<Entity> &entities) {
    std::vector<std::pair<int, Entity>> layered;
    layered.reserve(entities.size());
    bool mixed = false;
    for (const auto &entity : entities) {
        auto listener = cm.get_component<MouseInputListener<T>>(entity);
        int layer = listener != nullptr ? listener->layer : 0;
        if (!layered.empty() && layer != layered.front().first)
            mixed = true;
        layered.push_back({layer, entity});
    }
    if (mixed)
        std::stable_sort(layered.begin(), layered.end(),
                         [](const auto &a, const auto &b) { return a.first > b.first; });
    std::vector<Entity> ordered;
    ordered.reserve(layered.size());
    for (const auto &[layer, entity] : layered)
        ordered.push_back(entity);
    return ordered;
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

    // A copy, in layer order, so that handlers which remove entities (via
    // TE::remove_entity) don't invalidate the iterator.  notify_listener already
    // null-checks the component, so removed entities are silently skipped.
    auto entities = in_layer_order<MouseEvent>(cm, registered_entities);
    for (const auto &entity : entities)
        notify_listener(cm, event, entity);
    delete event;
}

void MouseInputSystem::mouse_release_handler(ComponentManager &cm,
                                              MouseEvent *event) {
    auto entities = in_layer_order<MouseEvent>(cm, registered_entities);
    for (const auto &entity : entities)
        notify_listener(cm, event, entity);
    delete event;
}

void MouseInputSystem::mouse_move_handler(ComponentManager &cm,
                                           MouseMoveEvent *event) {
    auto entities = in_layer_order<MouseMoveEvent>(cm, registered_move_entities);
    for (const auto &entity : entities) {
        notify_listener(cm, event, entity);
        if (event->picked_up)
            break;
    }
    delete event;
}

void MouseInputSystem::mouse_scroll_handler(ComponentManager &cm,
                                             MouseEvent *event) {
    auto entities = in_layer_order<MouseEvent>(cm, registered_entities);
    for (const auto &entity : entities) {
        notify_listener(cm, event, entity);
        if (event->picked_up)
            break;
    }
    delete event;
}
