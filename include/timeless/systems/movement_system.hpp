#pragma once
#include <glm/glm.hpp>
#include <array>
#include "timeless/components/transform.hpp"
#include "timeless/systems/system.hpp"

class MovementSystem : public System
{
private:
    enum Keys
    {
        w,
        a,
        s,
        d,
        escape,
        up,
        down,
        left,
        right
    };

    glm::vec2 x_bounds;
    glm::vec2 y_bounds;
    float walk_speed;
    std::array<bool, 10> keysPressed = {false, false, false, false, false, false, false, false, false, false};

    // Smoothed camera pan state. The camera is driven by a velocity that eases
    // toward the velocity the current input asks for, so key presses/releases
    // and edge-scroll entry/exit ramp up and down instead of snapping.
    glm::vec2 pan_velocity = glm::vec2(0.0f);
    float zoom_target;
    float last_applied_zoom;

public:
    Entity camera;

    // World units per second the camera pans at ZOOM == 1.0. The actual speed is
    // scaled by the zoom level, so panning covers a constant distance *on screen*
    // regardless of how far in you are: slow when zoomed in, fast when zoomed out.
    float camera_speed = 900.0f;
    // How strongly pan speed follows zoom. 1.0 = exactly screen-constant;
    // lower values take the edge off panning at the extremes.
    float zoom_pan_exponent = 1.0f;
    // Exponential smoothing rates (1/seconds). Higher = snappier, lower = floatier.
    float pan_smoothing = 14.0f;
    float zoom_smoothing = 12.0f;
    // Multiplicative zoom per scroll notch, so every notch is the same
    // perceived step at any zoom level.
    float zoom_step = 1.12f;
    glm::vec2 zoom_limits = glm::vec2(0.2f, 4.0f);
    // How far from the window edge (in pixels) edge scrolling starts ramping up.
    float edge_margin = 40.0f;

    MovementSystem();
    void register_camera(Entity c);
    glm::vec4 calculate_collider(glm::vec3 position, float width, float height);
    void move_right(ComponentManager &cm);
    void move_left(ComponentManager &cm);
    void move_down(ComponentManager &cm);
    void move_up(ComponentManager &cm);

    // Nudge the zoom target by a number of scroll notches (positive = zoom in).
    // The visible TESettings::ZOOM eases toward the target in update().
    void zoom_by(float notches);
    // Ease toward an absolute zoom level.
    void zoom_to(float zoom);
    // Jump straight to a zoom level, cancelling any in-flight easing.
    void set_zoom(float zoom);
    float get_zoom_target() const { return zoom_target; }

    void update(ComponentManager &cm, WindowManager &wm, float delta_time = 1.0f / 60.0f);
};
