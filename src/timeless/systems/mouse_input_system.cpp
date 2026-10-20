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
    // Front to back by z. Entities without a Transform go after all the
    // others, keeping their order: treating them as equal to everything
    // isn't a consistent ordering, which std::stable_sort needs.
    std::stable_sort(registered_entities.begin(), registered_entities.end(),
                     [&](const Entity &a, const Entity &b) {
                         auto trans_a = cm.get_component<Transform>(a);
                         auto trans_b = cm.get_component<Transform>(b);
                         if (trans_a == nullptr || trans_b == nullptr)
                             return trans_a != nullptr && trans_b == nullptr;
                         return trans_a->position.z > trans_b->position.z;
                     });
}

template <typename T>
std::vector<MouseInputSystem::Ordered<T>>
MouseInputSystem::in_layer_order(ComponentManager &cm,
                                 const std::vector<Entity> &entities) {
    std::vector<Ordered<T>> ordered;
    ordered.reserve(entities.size());
    bool mixed = false;
    for (const auto &entity : entities) {
        auto listener = cm.get_component<MouseInputListener<T>>(entity);
        if (listener == nullptr)
            continue;
        if (!ordered.empty() && listener->layer != ordered.front().layer)
            mixed = true;
        ordered.push_back({listener->layer, entity, listener});
    }
    if (mixed)
        std::stable_sort(ordered.begin(), ordered.end(),
                         [](const auto &a, const auto &b) { return a.layer > b.layer; });
    return ordered;
}

// Tells the listeners in `ordered` about `event`, in that order, stopping
// once one picks it up if `stop_when_picked_up`. One removed by an earlier
// handler (TE::remove_entity) has expired and is skipped.
template <typename T, typename Ordered>
static void dispatch(const std::vector<Ordered> &ordered, T *event,
                     bool stop_when_picked_up) {
    for (const auto &o : ordered) {
        auto listener = o.listener.lock();
        if (listener == nullptr)
            continue;
        listener->on_click_handler(event, o.entity, 0);
        if (stop_when_picked_up && event->picked_up)
            break;
    }
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

// Clicks and releases reach every listener, even once one has picked them
// up: each listener checks event->picked_up itself.
void MouseInputSystem::mouse_click_handler(ComponentManager &cm,
                                            MouseEvent *event) {
    auto cam = cm.get_component<Camera>(camera);

    std::string et = event->eventType;
    event->eventType = "Global" + et;

    dispatch(in_layer_order<MouseEvent>(cm, registered_entities), event, false);
    delete event;
}

void MouseInputSystem::mouse_release_handler(ComponentManager &cm,
                                              MouseEvent *event) {
    dispatch(in_layer_order<MouseEvent>(cm, registered_entities), event, false);
    delete event;
}

// Moves and scrolls stop at the first listener that picks them up.
void MouseInputSystem::mouse_move_handler(ComponentManager &cm,
                                           MouseMoveEvent *event) {
    dispatch(in_layer_order<MouseMoveEvent>(cm, registered_move_entities), event, true);
    delete event;
}

void MouseInputSystem::mouse_scroll_handler(ComponentManager &cm,
                                             MouseEvent *event) {
    dispatch(in_layer_order<MouseEvent>(cm, registered_entities), event, true);
    delete event;
}
