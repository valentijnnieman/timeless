#pragma once
#include "timeless/event.hpp"
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "timeless/input.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/systems/mouse_input_system.hpp"
#include "timeless/systems/event_system.hpp"

// Opaque platform handle: owns the backend window + GL context. Defined only
// in window_manager.cpp so SDL never appears in a public header.
struct PlatformWindow;

class WindowManager {
private:
  float quad_verts[24] = {
      // positions   // texCoords
      -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
      0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

      -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
      1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

public:
  std::vector<unsigned int> framebuffers;
  std::vector<unsigned int> textures;
  std::vector<unsigned int> rbos;
  std::vector<std::shared_ptr<Shader>> screen_shaders;
  bool running = true;

  int selected_screen_shader = 0;

  std::shared_ptr<ComponentManager> cm;
  std::shared_ptr<MouseInputSystem> mis;
  std::shared_ptr<EventSystem> es;

  glm::vec2 mouse_position;
  glm::vec2 raw_mouse_position;
  glm::vec2 shader_mouse_position;

  // --- Touch gestures (iOS / any SDL touch device) ---
  // Active fingers keyed by SDL_FingerID (stored as long long so SDL types stay
  // out of this header), in window pixels. When exactly two are down we treat
  // it as a pan+pinch gesture and accumulate the deltas below; callers drain
  // them each frame via consume_touch_pan()/consume_touch_pinch().
  std::map<long long, glm::vec2> active_touches;
  bool two_finger_active = false;       // had exactly 2 fingers last update
  glm::vec2 gesture_centroid{0.0f};     // last two-finger midpoint (pixels)
  float gesture_spread = 0.0f;          // last distance between the two fingers
  glm::vec2 touch_pan_accum{0.0f};      // unconsumed centroid movement (pixels)
  float touch_pinch_accum = 0.0f;       // unconsumed spread change (pixels)

  unsigned int ScreenVAO, ScreenVBO;

  // Backend window + GL context, hidden behind an opaque handle (PIMPL).
  std::unique_ptr<PlatformWindow> platform;
  bool quit_requested = false;

  WindowManager(std::shared_ptr<ComponentManager> cm,
                std::shared_ptr<MouseInputSystem> mis);
  ~WindowManager();
  void cleanup();

  // --- Main-loop / platform surface (backend-agnostic) ---
  void swap_buffers();
  void poll_events();              // pumps backend events into engine handlers
  bool should_close();             // SDL_QUIT or TE::quit() was called
  bool is_key_pressed(TE::Key key); // polling query (replaces glfwGetKey)
  bool is_mouse_button_pressed(TE::MouseButton button); // replaces glfwGetMouseButton
  glm::vec2 get_cursor_position();  // current cursor in window pixels
  void set_cursor_visible(bool visible); // show/hide the OS cursor
  void set_fullscreen(bool enabled);     // borderless-desktop fullscreen toggle
  void set_window_size(int width, int height); // resize + recenter the OS window

  // Two-finger gesture deltas accumulated since the last call (and reset by it).
  // Pan is centroid movement in window pixels; pinch is the change in finger
  // spread (positive = fingers moved apart). Zero when fewer than two fingers
  // are down. Poll these once per frame, like is_key_pressed.
  glm::vec2 consume_touch_pan();
  float consume_touch_pinch();

  void add_framebuffer(std::shared_ptr<Shader> shader, int width = TESettings::SCREEN_X, int height = TESettings::SCREEN_Y, bool add_screen_shader = true);
  void select_framebuffer(size_t idx, bool clear = true);
  void resize_framebuffers(int new_width, int new_height);
  void render_framebuffer_as_quad(size_t idx, bool clear = true, int tick = 0, bool to_screen = false);
  void render_background_quad(std::shared_ptr<Shader> shader);
  void set_shader_time(std::shared_ptr<Shader> shader);
  void set_shader_mouse_position(glm::vec2 mouse_pos);

  void handle_resize(int width, int height);
  void update_touch_gesture(); // recompute two-finger pan/pinch from active_touches
  void mouse_move_handler(MouseMoveEvent *event);
  void mouse_click_handler(MouseEvent *event);
  void mouse_release_handler(MouseEvent *event);
  void mouse_scroll_handler(MouseEvent *event);
};
