#include "timeless/systems/movement_system.hpp"
#include "timeless/managers/window_manager.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace {
// Camera turn about the vertical axis in degrees: 0 for the default view
// looking toward -Y (matches RenderingSystem::yaw_of).
float camera_yaw_degrees(const std::shared_ptr<Camera> &cam) {
    glm::vec3 f = cam->get_forward();
    if (std::abs(f.x) < 1e-6f && std::abs(f.y) < 1e-6f) return 0.0f;
    return glm::degrees(std::atan2(f.x, -f.y));
}

// Wrap an angle difference into [-180, 180) so turns take the short way.
float wrap_degrees(float a) {
    a = std::fmod(a + 180.0f, 360.0f);
    if (a < 0.0f) a += 360.0f;
    return a - 180.0f;
}

// The ground (z = 0) point under a window position, seen through `cam` at
// `zoom`; false if that ray runs parallel to the ground. Window pixels map to
// the viewport as in TE::clicked_on_perspective.
bool ground_under(const std::shared_ptr<Camera> &cam, glm::vec2 window_pos,
                  float zoom, glm::vec3 &out) {
    glm::vec4 viewport(0.0f, 0.0f, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
    glm::mat4 view = cam->get_view_matrix();
    glm::mat4 proj = cam->get_projection_matrix(TESettings::VIEWPORT_X,
                                                TESettings::VIEWPORT_Y, zoom);
    float x = window_pos.x * float(TESettings::VIEWPORT_X) / TESettings::WINDOW_X;
    float y = viewport.w - window_pos.y * float(TESettings::VIEWPORT_Y) / TESettings::WINDOW_Y;
    glm::vec3 near_point = glm::unProject(glm::vec3(x, y, 0.0f), view, proj, viewport);
    glm::vec3 far_point  = glm::unProject(glm::vec3(x, y, 1.0f), view, proj, viewport);
    glm::vec3 dir = far_point - near_point;
    if (std::abs(dir.z) < 1e-6f) return false;
    out = near_point + dir * (-near_point.z / dir.z);
    return true;
}

// Move the camera over the ground, keeping its height.
void shift_camera(const std::shared_ptr<Camera> &cam, glm::vec3 by) {
    glm::vec3 pos = cam->get_position();
    cam->set_position(glm::vec3(pos.x + by.x, pos.y + by.y, pos.z));
}
}

MovementSystem::MovementSystem()
    : x_bounds(glm::vec2(-TESettings::SCREEN_X, TESettings::SCREEN_X)),
      y_bounds(glm::vec2(-TESettings::SCREEN_Y, TESettings::SCREEN_Y)),
      walk_speed(1.0f), zoom_target(TESettings::ZOOM),
      last_applied_zoom(TESettings::ZOOM) {
#if TARGET_OS_IOS
    edge_margin = 0.0f;
#endif
}

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

void MovementSystem::begin_drag(glm::vec2 window_pos) {
    drag_pending = true;
    drag_moved   = false;
    drag_start   = window_pos;
    drag_last    = window_pos;
}

void MovementSystem::cancel_drag() {
    drag_pending = false;
    drag_moved   = false;
    pinching     = false;
}

void MovementSystem::update_drag(ComponentManager &cm, WindowManager &wm) {
    auto cam = cm.get_component<Camera>(camera);

    glm::vec2 centroid;
    float spread;
    if (wm.get_touch_gesture(centroid, spread)) {
        // A second finger turns the first finger's drag into a pinch.
        if (!drag_pending && !pinching) return;
        if (pinching && cam != nullptr && spread > 1.0f && pinch_spread > 1.0f) {
            // Zoom by how much the fingers spread (apart = in), then move the
            // camera so the ground that was between the fingers still is.
            glm::vec3 before, after;
            bool found = ground_under(cam, pinch_centroid, TESettings::ZOOM, before);
            set_zoom(TESettings::ZOOM * pinch_spread / spread);
            if (found && ground_under(cam, centroid, TESettings::ZOOM, after))
                shift_camera(cam, before - after);
        }
        pinching       = true;
        drag_pending   = false;
        drag_moved     = false;
        pinch_centroid = centroid;
        pinch_spread   = spread;
        return;
    }
    pinching = false;

    if (!drag_pending) return;
    if (!wm.is_mouse_button_pressed(TE::MouseButton::Left)) {
        cancel_drag();
        return;
    }
    glm::vec2 cursor = wm.get_cursor_position();
    if (!drag_moved) {
        if (glm::length(cursor - drag_start) < drag_threshold) return;
        drag_moved = true;
    }
    glm::vec3 before, after;
    if (cam != nullptr && ground_under(cam, drag_last, TESettings::ZOOM, before) &&
        ground_under(cam, cursor, TESettings::ZOOM, after))
        shift_camera(cam, before - after);
    drag_last = cursor;
}

void MovementSystem::turn_camera(float degrees, ComponentManager &cm) {
    auto cam = cm.get_component<Camera>(camera);
    if (cam == nullptr) return;
    // Stack presses during a turn; otherwise start from the nearest whole
    // step so the view always lands square on the grid.
    if (!rotating)
        yaw_target = std::round(camera_yaw_degrees(cam) / turn_step) * turn_step;
    yaw_target += degrees;
    rotating = true;
}

void MovementSystem::update_rotation(ComponentManager &cm, float dt) {
    if (!rotating) return;
    auto cam = cm.get_component<Camera>(camera);
    if (cam == nullptr) { rotating = false; return; }

    float diff = wrap_degrees(yaw_target - camera_yaw_degrees(cam));
    float step = diff * (1.0f - std::exp(-rotate_smoothing * dt));
    if (std::abs(diff) < 0.05f) {
        step = diff;
        rotating = false;
    }

    // Turn about the ground point at the centre of the view, so whatever
    // you're looking at stays put while the city swings around it.
    glm::vec3 pos = cam->get_position();
    glm::vec3 f = cam->get_forward();
    glm::vec3 pivot = pos;
    if (std::abs(f.z) > 1e-3f)
        pivot = pos + f * (-pos.z / f.z);

    glm::quat turn = glm::angleAxis(glm::radians(step), glm::vec3(0, 0, 1));
    cam->set_rotation(turn * cam->get_rotation());
    cam->set_position(pivot + turn * (pos - pivot));
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

    // Q/E turn the camera one step per press (edge-triggered, not held).
    if (wm.is_key_pressed(TE::Key::Q)) {
        if (!keysPressed[q]) turn_camera(turn_step, cm);
        keysPressed[q] = true;
    } else {
        keysPressed[q] = false;
    }
    if (wm.is_key_pressed(TE::Key::E)) {
        if (!keysPressed[e]) turn_camera(-turn_step, cm);
        keysPressed[e] = true;
    } else {
        keysPressed[e] = false;
    }
    update_rotation(cm, dt);
    update_drag(cm, wm);

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
        // Not while dragging, which may well take the cursor near an edge.
        if (inside && edge_margin > 0.0f && !drag_pending && !pinching) {
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

        // Pan relative to the camera's facing so W is always "up the screen"
        // after a Q/E turn: back = away from where the camera looks (screen
        // down), right = 90 degrees from that. In the default view these are
        // world +Y and +X, the axes panning always used.
        glm::vec2 right(1.0f, 0.0f), back(0.0f, 1.0f);
        glm::vec3 f = main_camera->get_forward();
        glm::vec2 flat(-f.x, -f.y);
        if (glm::length(flat) > 1e-4f) {
            back  = glm::normalize(flat);
            right = glm::vec2(back.y, -back.x);
        }
        glm::vec2 world_velocity = pan_velocity.x * right + pan_velocity.y * back;

        glm::vec3 pos = main_camera->get_position();
        main_camera->set_position(glm::vec3(pos.x + world_velocity.x * dt,
                                            pos.y + world_velocity.y * dt,
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
