#include "timeless/systems/keyboard_input_system.hpp"
#include "timeless/managers/window_manager.hpp"

void KeyboardInputSystem::notify_listener(ComponentManager &cm,
                                          KeyboardEvent *event, Entity entity) {
    auto listener = cm.get_component<KeyboardInputListener>(entity);
    listener->on_input_handler(event, entity, 0);
    delete event;
}

glm::vec4 KeyboardInputSystem::calculate_collider(glm::vec3 position,
                                                   float width, float height) {
    float x = position.x;
    float y = position.y;
    return glm::vec4(x, x + width, y, y + height);
}

void KeyboardInputSystem::update(ComponentManager &cm, WindowManager &wm) {
    if (wm.is_key_pressed(TE::Key::Escape)) {
        keysPressed[escape] = true;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("PressEscape"), entity);
    }
    if (!wm.is_key_pressed(TE::Key::Escape) && keysPressed[escape]) {
        keysPressed[escape] = false;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("ReleaseEscape"), entity);
    }
    if (wm.is_key_pressed(TE::Key::Space) && !keysPressed[space]) {
        keysPressed[space] = true;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("PressSpace"), entity);
    }
    if (!wm.is_key_pressed(TE::Key::Space) && keysPressed[space]) {
        keysPressed[space] = false;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("ReleaseSpace"), entity);
    }
    if (wm.is_key_pressed(TE::Key::Enter) && !keysPressed[enter]) {
        keysPressed[enter] = true;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("PressEnter"), entity);
    }
    if (!wm.is_key_pressed(TE::Key::Enter) && keysPressed[enter]) {
        keysPressed[enter] = false;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("ReleaseEnter"), entity);
    }
    if (wm.is_key_pressed(TE::Key::F) && !keysPressed[f]) {
        keysPressed[f] = true;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("PressF"), entity);
    }
    if (!wm.is_key_pressed(TE::Key::F) && keysPressed[f]) {
        keysPressed[f] = false;
        for (auto &entity : registered_entities)
            notify_listener(cm, new KeyboardEvent("ReleaseF"), entity);
    }
}
