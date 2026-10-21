#pragma once
#include "timeless/event.hpp"
#include "timeless/gl.hpp"
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
  // Colour attachments past the first, per framebuffer (GL_COLOR_ATTACHMENT1
  // onwards), and how many each framebuffer asked for so a resize recreates
  // them. render_framebuffer_as_quad binds them to texture units 1.. as
  // screenTexture1, screenTexture2, ...
  std::vector<std::vector<unsigned int>> extra_textures;
  std::vector<int> extra_attachment_counts;
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
  // out of this header), in window pixels. When exactly two are down they
  // make a pan+pinch gesture, read with get_touch_gesture().
  std::map<long long, glm::vec2> active_touches;
  bool two_finger_active = false;       // exactly 2 fingers are down
  glm::vec2 gesture_centroid{0.0f};     // their midpoint (pixels)
  float gesture_spread = 0.0f;          // the distance between them (pixels)

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

  // Whether exactly two fingers are down, and if so their midpoint and the
  // distance between them, in window pixels. Poll once per frame, like
  // is_key_pressed: the change from one frame to the next is the pan + pinch.
  bool get_touch_gesture(glm::vec2 &centroid, float &spread) const;

  void add_framebuffer(std::shared_ptr<Shader> shader, int width = TESettings::RENDER_X, int height = TESettings::RENDER_Y, bool add_screen_shader = true, int extra_color_attachments = 0);
  // Clearing also zeroes the extra colour attachments (not SCREEN_COLOR).
  void select_framebuffer(size_t idx, bool clear = true);
  // Which colour attachments of the bound framebuffer draws write to: the
  // first `count` (1 = only the main colour). Shaders that don't write an
  // attachment must not draw with it enabled, or its contents are undefined.
  void set_draw_attachments(size_t idx, int count);
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
