#include "timeless/systems/movement_system.hpp"
#include "timeless/managers/window_manager.hpp"

MovementSystem::MovementSystem()
    : x_bounds(glm::vec2(-TESettings::SCREEN_X, TESettings::SCREEN_X)),
      y_bounds(glm::vec2(-TESettings::SCREEN_Y, TESettings::SCREEN_Y)),
      walk_speed(1.0f), camera_speed(15.0f) {}

void MovementSystem::register_camera(Entity c) { camera = c; }

glm::vec4 MovementSystem::calculate_collider(glm::vec3 position, float width,
                                              float height) {
    float x = position.x;
    float y = position.y;
    return glm::vec4(x, x + width, y, y + height);
}

void MovementSystem::move_right(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
        auto sprite    = cm.get_component<Sprite>(entity);
        auto transform = cm.get_component<Transform>(entity);
        auto collider  = cm.get_component<Collider>(entity);
        if (transform->position.x + walk_speed < x_bounds[1]) {
            glm::vec4 colPos = calculate_collider(
                glm::vec3(transform->position.x + walk_speed,
                          transform->position.y, 0.0),
                transform->width, transform->height);
            collider->setBounds(colPos);
            int grid_layer = 0;
            if (grid_layer != 1) {
                transform->position.x += walk_speed;
                sprite->animating = true;
                sprite->flip      = true;
            }
        }
    }
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera != nullptr)
        main_camera->set_position(glm::vec3(
            main_camera->get_position().x + camera_speed,
            main_camera->get_position().y,
            main_camera->get_position().z));
}

void MovementSystem::move_left(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
        auto sprite    = cm.get_component<Sprite>(entity);
        auto transform = cm.get_component<Transform>(entity);
        auto collider  = cm.get_component<Collider>(entity);
        if (transform->position.x - walk_speed > x_bounds[0]) {
            glm::vec4 colPos = calculate_collider(
                glm::vec3(transform->position.x - walk_speed,
                          transform->position.y, 0.0),
                transform->width, transform->height);
            collider->setBounds(colPos);
            int grid_layer = 0;
            if (grid_layer != 1) {
                transform->position.x -= walk_speed;
                sprite->animating = true;
                sprite->flip      = true;
            }
        }
    }
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera != nullptr)
        main_camera->set_position(glm::vec3(
            main_camera->get_position().x - camera_speed,
            main_camera->get_position().y,
            main_camera->get_position().z));
}

void MovementSystem::move_down(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
        auto sprite    = cm.get_component<Sprite>(entity);
        auto transform = cm.get_component<Transform>(entity);
        auto collider  = cm.get_component<Collider>(entity);
        if (transform->position.y + walk_speed < y_bounds[1]) {
            glm::vec4 colPos = calculate_collider(
                glm::vec3(transform->position.x,
                          transform->position.y + walk_speed, 0.0),
                transform->width, transform->height);
            collider->setBounds(colPos);
            int grid_layer = 0;
            if (grid_layer != 1) {
                transform->position.y += walk_speed;
                sprite->animating = true;
            }
        }
    }
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera != nullptr)
        main_camera->set_position(glm::vec3(
            main_camera->get_position().x,
            main_camera->get_position().y + camera_speed,
            main_camera->get_position().z));
}

void MovementSystem::move_up(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
        auto sprite    = cm.get_component<Sprite>(entity);
        auto transform = cm.get_component<Transform>(entity);
        auto collider  = cm.get_component<Collider>(entity);
        if (transform->position.y - walk_speed > y_bounds[0]) {
            glm::vec4 colPos = calculate_collider(
                glm::vec3(transform->position.x,
                          transform->position.y + walk_speed, 0.0),
                transform->width, transform->height);
            collider->setBounds(colPos);
            int grid_layer = 0;
            if (grid_layer != 1) {
                transform->position.y += walk_speed;
                sprite->animating = true;
            }
        }
    }
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera != nullptr)
        main_camera->set_position(glm::vec3(
            main_camera->get_position().x,
            main_camera->get_position().y - camera_speed,
            main_camera->get_position().z));
}

void MovementSystem::update(ComponentManager &cm, WindowManager &wm) {
    if (wm.is_key_pressed(te::Key::D)) { move_right(cm); keysPressed[d] = true; }
    if (wm.is_key_pressed(te::Key::A)) { move_left(cm);  keysPressed[a] = true; }
    if (wm.is_key_pressed(te::Key::W)) { move_up(cm);    keysPressed[w] = true; }
    if (wm.is_key_pressed(te::Key::S)) { move_down(cm);  keysPressed[s] = true; }
    if (wm.is_key_pressed(te::Key::Up))    { move_up(cm);    keysPressed[up]    = true; }
    if (wm.is_key_pressed(te::Key::Down))  { move_down(cm);  keysPressed[down]  = true; }
    if (wm.is_key_pressed(te::Key::Left))  { move_left(cm);  keysPressed[left]  = true; }
    if (wm.is_key_pressed(te::Key::Right)) { move_right(cm); keysPressed[right] = true; }
    if (wm.is_key_pressed(te::Key::Escape)) keysPressed[escape] = true;

    if (!wm.is_key_pressed(te::Key::Escape) && keysPressed[escape]) keysPressed[escape] = false;
    if (!wm.is_key_pressed(te::Key::A)      && keysPressed[a])      keysPressed[a]      = false;
    if (!wm.is_key_pressed(te::Key::W)      && keysPressed[w])      keysPressed[w]      = false;
    if (!wm.is_key_pressed(te::Key::S)      && keysPressed[s])      keysPressed[s]      = false;
    if (!wm.is_key_pressed(te::Key::D)      && keysPressed[d])      keysPressed[d]      = false;
    if (!wm.is_key_pressed(te::Key::Up)     && keysPressed[up])     keysPressed[up]     = false;
    if (!wm.is_key_pressed(te::Key::Down)   && keysPressed[down])   keysPressed[down]   = false;
    if (!wm.is_key_pressed(te::Key::Left)   && keysPressed[left])   keysPressed[left]   = false;
    if (!wm.is_key_pressed(te::Key::Right)  && keysPressed[right])  keysPressed[right]  = false;

    // Edge scrolling
    {
        glm::vec2 cursor = wm.get_cursor_position();
        int win_w = TESettings::WINDOW_X;
        int win_h = TESettings::WINDOW_Y;
        constexpr int edge_margin = 20;
        if (cursor.x >= win_w - edge_margin) move_right(cm);
        if (cursor.x <= edge_margin)         move_left(cm);
        if (cursor.y <= edge_margin)         move_up(cm);
        if (cursor.y >= win_h - edge_margin) move_down(cm);
    }
}
