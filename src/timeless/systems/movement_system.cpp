#include "timeless/systems/movement_system.hpp"
#include "timeless/managers/window_manager.hpp"
#include <algorithm>
#include <cmath>

MovementSystem::MovementSystem()
    : x_bounds(glm::vec2(-TESettings::SCREEN_X, TESettings::SCREEN_X)),
      y_bounds(glm::vec2(-TESettings::SCREEN_Y, TESettings::SCREEN_Y)),
      walk_speed(1.0f), zoom_target(TESettings::ZOOM),
      last_applied_zoom(TESettings::ZOOM) {}

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
}

void MovementSystem::zoom_by(float notches) {
    // Multiplicative rather than additive: a notch is always the same relative
    // change, so zooming feels identical whether you're at 0.2 or 4.0.
    // Larger ZOOM = wider ortho extents = further out, hence the negated exponent.
    zoom_target = std::clamp(zoom_target * std::pow(zoom_step, -notches),
                             zoom_limits[0], zoom_limits[1]);
}

void MovementSystem::zoom_to(float zoom) {
    zoom_target = std::clamp(zoom, zoom_limits[0], zoom_limits[1]);
}

void MovementSystem::set_zoom(float zoom) {
    zoom_target       = std::clamp(zoom, zoom_limits[0], zoom_limits[1]);
    TESettings::ZOOM  = zoom_target;
    last_applied_zoom = zoom_target;
}

void MovementSystem::update(ComponentManager &cm, WindowManager &wm,
                            float delta_time) {
    // Guard against pauses/breakpoints producing a huge step.
    float dt = std::clamp(delta_time, 0.0f, 0.1f);

    // -- gather pan input -------------------------------------------------
    // +x = right, +y = down in camera terms (matches the old move_* directions).
    glm::vec2 dir(0.0f);

    if (wm.is_key_pressed(TE::Key::D)) { move_right(cm); dir.x += 1.0f; keysPressed[d] = true; }
    if (wm.is_key_pressed(TE::Key::A)) { move_left(cm);  dir.x -= 1.0f; keysPressed[a] = true; }
    if (wm.is_key_pressed(TE::Key::W)) { move_up(cm);    dir.y -= 1.0f; keysPressed[w] = true; }
    if (wm.is_key_pressed(TE::Key::S)) { move_down(cm);  dir.y += 1.0f; keysPressed[s] = true; }
    if (wm.is_key_pressed(TE::Key::Up))    { move_up(cm);    dir.y -= 1.0f; keysPressed[up]    = true; }
    if (wm.is_key_pressed(TE::Key::Down))  { move_down(cm);  dir.y += 1.0f; keysPressed[down]  = true; }
    if (wm.is_key_pressed(TE::Key::Left))  { move_left(cm);  dir.x -= 1.0f; keysPressed[left]  = true; }
    if (wm.is_key_pressed(TE::Key::Right)) { move_right(cm); dir.x += 1.0f; keysPressed[right] = true; }
    if (wm.is_key_pressed(TE::Key::Escape)) keysPressed[escape] = true;

    if (!wm.is_key_pressed(TE::Key::Escape) && keysPressed[escape]) keysPressed[escape] = false;
    if (!wm.is_key_pressed(TE::Key::A)      && keysPressed[a])      keysPressed[a]      = false;
    if (!wm.is_key_pressed(TE::Key::W)      && keysPressed[w])      keysPressed[w]      = false;
    if (!wm.is_key_pressed(TE::Key::S)      && keysPressed[s])      keysPressed[s]      = false;
    if (!wm.is_key_pressed(TE::Key::D)      && keysPressed[d])      keysPressed[d]      = false;
    if (!wm.is_key_pressed(TE::Key::Up)     && keysPressed[up])     keysPressed[up]     = false;
    if (!wm.is_key_pressed(TE::Key::Down)   && keysPressed[down])   keysPressed[down]   = false;
    if (!wm.is_key_pressed(TE::Key::Left)   && keysPressed[left])   keysPressed[left]   = false;
    if (!wm.is_key_pressed(TE::Key::Right)  && keysPressed[right])  keysPressed[right]  = false;

    // Edge scrolling, ramped by how deep into the margin the cursor is instead
    // of switching on at full speed the moment it crosses the threshold.
    {
        glm::vec2 cursor = wm.get_cursor_position();
        float win_w = static_cast<float>(TESettings::WINDOW_X);
        float win_h = static_cast<float>(TESettings::WINDOW_Y);
        bool inside = cursor.x >= 0.0f && cursor.x <= win_w &&
                      cursor.y >= 0.0f && cursor.y <= win_h;
        if (inside && edge_margin > 0.0f) {
            auto ramp = [this](float distance_into_margin) {
                return std::clamp(distance_into_margin / edge_margin, 0.0f, 1.0f);
            };
            dir.x += ramp(cursor.x - (win_w - edge_margin));
            dir.x -= ramp(edge_margin - cursor.x);
            dir.y -= ramp(edge_margin - cursor.y);
            dir.y += ramp(cursor.y - (win_h - edge_margin));
        }
    }

    // Clamp instead of normalize: keeps diagonals from being ~1.4x faster while
    // still letting a partially-ramped edge scroll move at partial speed.
    float dir_len = glm::length(dir);
    if (dir_len > 1.0f) dir /= dir_len;

    // -- pan ---------------------------------------------------------------
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera != nullptr) {
        // Scale by zoom so the world slides past at a constant *screen* rate:
        // slow when zoomed in, fast when zoomed out.
        float zoom_scale = std::pow(std::max(TESettings::ZOOM, 0.0001f),
                                    zoom_pan_exponent);
        glm::vec2 target_velocity = dir * camera_speed * zoom_scale;

        // Frame-rate independent exponential approach: the fraction we close
        // per frame depends on dt, so the feel is identical at 30 or 144 fps.
        float t      = 1.0f - std::exp(-pan_smoothing * dt);
        pan_velocity = glm::mix(pan_velocity, target_velocity, t);
        if (glm::length(pan_velocity) < 0.01f) pan_velocity = glm::vec2(0.0f);

        glm::vec3 pos = main_camera->get_position();
        main_camera->set_position(glm::vec3(pos.x + pan_velocity.x * dt,
                                            pos.y + pan_velocity.y * dt,
                                            pos.z));
    }

    // -- zoom --------------------------------------------------------------
    // Anything that writes TESettings::ZOOM directly (cutscenes, focus helpers)
    // wins: adopt it as the new target rather than fighting it.
    if (std::abs(TESettings::ZOOM - last_applied_zoom) > 1e-4f)
        zoom_target = TESettings::ZOOM;

    float zt = 1.0f - std::exp(-zoom_smoothing * dt);
    TESettings::ZOOM = glm::mix(TESettings::ZOOM, zoom_target, zt);
    if (std::abs(TESettings::ZOOM - zoom_target) < 1e-4f)
        TESettings::ZOOM = zoom_target;
    last_applied_zoom = TESettings::ZOOM;
}
