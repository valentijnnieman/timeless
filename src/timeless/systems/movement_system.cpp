#include "timeless/systems/movement_system.hpp"

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

void MovementSystem::update(ComponentManager &cm, GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { move_right(cm); keysPressed[d] = true; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { move_left(cm);  keysPressed[a] = true; }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { move_up(cm);    keysPressed[w] = true; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { move_down(cm);  keysPressed[s] = true; }
    if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) { move_up(cm);    keysPressed[up]    = true; }
    if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) { move_down(cm);  keysPressed[down]  = true; }
    if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) { move_left(cm);  keysPressed[left]  = true; }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { move_right(cm); keysPressed[right] = true; }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) keysPressed[escape] = true;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE && keysPressed[escape]) keysPressed[escape] = false;
    if (glfwGetKey(window, GLFW_KEY_A)      == GLFW_RELEASE && keysPressed[a])      keysPressed[a]      = false;
    if (glfwGetKey(window, GLFW_KEY_W)      == GLFW_RELEASE && keysPressed[w])      keysPressed[w]      = false;
    if (glfwGetKey(window, GLFW_KEY_S)      == GLFW_RELEASE && keysPressed[s])      keysPressed[s]      = false;
    if (glfwGetKey(window, GLFW_KEY_D)      == GLFW_RELEASE && keysPressed[d])      keysPressed[d]      = false;
    if (glfwGetKey(window, GLFW_KEY_UP)     == GLFW_RELEASE && keysPressed[up])     keysPressed[up]     = false;
    if (glfwGetKey(window, GLFW_KEY_DOWN)   == GLFW_RELEASE && keysPressed[down])   keysPressed[down]   = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT)   == GLFW_RELEASE && keysPressed[left])   keysPressed[left]   = false;
    if (glfwGetKey(window, GLFW_KEY_RIGHT)  == GLFW_RELEASE && keysPressed[right])  keysPressed[right]  = false;

    // Edge scrolling
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        constexpr int edge_margin = 20;
        if (xpos >= TESettings::WINDOW_X - edge_margin) move_right(cm);
        if (xpos <= edge_margin)                        move_left(cm);
        if (ypos <= edge_margin)                        move_up(cm);
        if (ypos >= TESettings::WINDOW_Y - edge_margin) move_down(cm);
    }
}
