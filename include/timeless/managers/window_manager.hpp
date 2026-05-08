#pragma once
#include "timeless/event.hpp"
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "timeless/components/transform.hpp"
#include "timeless/systems/mouse_input_system.hpp"
#include "timeless/systems/event_system.hpp"

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

  unsigned int ScreenVAO, ScreenVBO;

  GLFWwindow *window;
  GLFWcursor *cursor;

  static void error_callback(int error, const char *description);

  WindowManager(std::shared_ptr<ComponentManager> cm,
                std::shared_ptr<MouseInputSystem> mis);
  void cleanup();

  void add_framebuffer(std::shared_ptr<Shader> shader, int width = TESettings::SCREEN_X, int height = TESettings::SCREEN_Y, bool add_screen_shader = true);
  void select_framebuffer(size_t idx, bool clear = true);
  void resize_framebuffers(int new_width, int new_height);
  void render_framebuffer_as_quad(size_t idx, bool clear = true, int tick = 0, bool to_screen = false);
  void set_shader_time(std::shared_ptr<Shader> shader);
  void set_shader_mouse_position(glm::vec2 mouse_pos);

  static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
  static void window_size_callback(GLFWwindow *window, int width, int height);
  void mouse_move_handler(MouseMoveEvent *event);
  void mouse_click_handler(MouseEvent *event);
  void mouse_release_handler(MouseEvent *event);
  void mouse_scroll_handler(MouseEvent *event);
  static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
};
