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
        right,
        q,
        e
    };

    glm::vec2 x_bounds;
    glm::vec2 y_bounds;
    float walk_speed;
    std::array<bool, 12> keysPressed = {};

    // Smoothed camera pan state. The camera is driven by a velocity that eases
    // toward the velocity the current input asks for, so key presses/releases
    // and edge-scroll entry/exit ramp up and down instead of snapping.
    glm::vec2 pan_velocity = glm::vec2(0.0f);
    float zoom_target;
    float last_applied_zoom;

    // Q/E camera turns: the yaw (degrees) the camera is easing toward, and
    // whether a turn is in progress. The current yaw is always read back from
    // the camera itself, so anything else moving the camera can't desync it.
    float yaw_target = 0.0f;
    bool rotating = false;

    // Drag panning: a drag begun with begin_drag() (pending) that has moved
    // past drag_threshold, and the cursor position it last panned to.
    bool drag_pending = false;
    bool drag_moved = false;
    glm::vec2 drag_start = glm::vec2(0.0f);
    glm::vec2 drag_last = glm::vec2(0.0f);
    // Pinch zoom: the two-finger midpoint and spread last frame.
    bool pinching = false;
    glm::vec2 pinch_centroid = glm::vec2(0.0f);
    float pinch_spread = 0.0f;

    void turn_camera(float degrees, ComponentManager &cm);
    void update_rotation(ComponentManager &cm, float dt);
    void update_drag(ComponentManager &cm, WindowManager &wm);

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
    float rotate_smoothing = 10.0f;
    // Degrees per Q/E press. Q turns one way, E the other.
    float turn_step = 90.0f;
    // Multiplicative zoom per scroll notch, so every notch is the same
    // perceived step at any zoom level.
    float zoom_step = 1.12f;
    glm::vec2 zoom_limits = glm::vec2(0.2f, 4.0f);
    // How far from the window edge (in pixels) edge scrolling starts ramping up.
    // 0 (off) on iOS, where the cursor stays wherever the last touch lifted.
    float edge_margin = 40.0f;
    // How far (window pixels) a drag moves before it pans, so a tap or a
    // click that wobbles a little is still just a click.
    float drag_threshold = 8.0f;

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

    // Drag the map with the left mouse button or a finger, and pinch-zoom
    // with two fingers. Both grab the map: the ground under the cursor, or
    // between the fingers, stays under it. Call begin_drag() on a press the
    // game decides is on the map (window pixels, MouseEvent::raw_position);
    // the drag follows the cursor until the button is released. A pinch only
    // starts from such a drag (its first finger), so pinching over the UI
    // leaves the map alone.
    void begin_drag(glm::vec2 window_pos);
    void cancel_drag();
    // Whether a drag is panning (it has passed drag_threshold).
    bool dragging() const { return drag_moved; }

    void update(ComponentManager &cm, WindowManager &wm, float delta_time = 1.0f / 60.0f);
};
