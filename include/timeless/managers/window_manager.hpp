#pragma once
#include "timeless/event.hpp"
#include <GLFW/glfw3.h>
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <iostream>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "timeless/components/transform.hpp"
#include "timeless/systems/mouse_input_system.hpp"

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

  glm::vec2 mouse_position;
  glm::vec2 raw_mouse_position;
  glm::vec2 shader_mouse_position;

  unsigned int ScreenVAO, ScreenVBO;

  GLFWwindow *window;
  GLFWcursor *cursor;

  static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
  }

  WindowManager(std::shared_ptr<ComponentManager> cm,
                std::shared_ptr<MouseInputSystem> mis)
      : cm(cm), mis(mis) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    if (TESettings::FULLSCREEN)
      window = glfwCreateWindow(TESettings::SCREEN_X, TESettings::SCREEN_Y,
                                "Timeless", glfwGetPrimaryMonitor(), NULL);
    else
      window = glfwCreateWindow(TESettings::SCREEN_X, TESettings::SCREEN_Y,
                                "Timeless", NULL, NULL);

    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowUserPointer(window, this);

    glfwSwapInterval(1);

    cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    glfwSetCursor(window, cursor);

#ifdef __EMSCRIPTEN__
#else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
    }
#endif
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "OpenGL Initialized!" << std::endl;

    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, &scroll_callback);

    glGenVertexArrays(1, &ScreenVAO);
    glGenBuffers(1, &ScreenVBO);
    glBindVertexArray(ScreenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ScreenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  void cleanup() {
    std::cout << "Destroying WindowManager..." << std::endl;
    glfwDestroyWindow(window);
    glfwDestroyCursor(cursor);
    glDeleteVertexArrays(1, &ScreenVAO);
    glDeleteBuffers(1, &ScreenVBO);
    for (auto fbo : framebuffers) glDeleteFramebuffers(1, &fbo);
    for (auto tex : textures) glDeleteTextures(1, &tex);
    for (auto rbo : rbos) glDeleteRenderbuffers(1, &rbo);
  }

  // ~WindowManager() {
  //   std::cout << "Destroying WindowManager..." << std::endl;
  //   glfwDestroyWindow(window);
  //   glfwDestroyCursor(cursor);
  //   glDeleteVertexArrays(1, &ScreenVAO);
  //   glDeleteBuffers(1, &ScreenVBO);
  //   for (auto fbo : framebuffers) glDeleteFramebuffers(1, &fbo);
  //   for (auto tex : textures) glDeleteTextures(1, &tex);
  //   for (auto rbo : rbos) glDeleteRenderbuffers(1, &rbo);
  // }

  void add_framebuffer(std::shared_ptr<Shader> shader, int width = TESettings::SCREEN_X, int height = TESettings::SCREEN_Y, bool add_screen_shader = true) {
    unsigned int fbo, tex, rbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Could not create Framebuffer!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    framebuffers.push_back(fbo);
    textures.push_back(tex);
    rbos.push_back(rbo);
    if(add_screen_shader)
      screen_shaders.push_back(shader);
  }

  void select_framebuffer(size_t idx, bool clear = true) {
    if (idx < framebuffers.size()) {
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[idx]);
      glViewport(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
      if (clear) {
        glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g,
                     TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      }
    }
  }

  void resize_framebuffers(int new_width, int new_height) {
    for (size_t i = 0; i < framebuffers.size(); ++i) {
      // Delete old resources
      glDeleteFramebuffers(1, &framebuffers[i]);
      glDeleteTextures(1, &textures[i]);
      glDeleteRenderbuffers(1, &rbos[i]);
    }
    framebuffers.clear();
    textures.clear();
    rbos.clear();

    // Recreate framebuffers with new size
    for (size_t i = 0; i < screen_shaders.size(); ++i) {
      add_framebuffer(screen_shaders[i], new_width, new_height, false);
    }
  }

  void render_framebuffer_as_quad(size_t idx, bool clear = true, int tick = 0) {
    if (idx >= screen_shaders.size() || idx >= textures.size()) return;
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
    if (clear) {
      glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g,
                   TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    screen_shaders[idx]->use();

    glBindVertexArray(ScreenVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[idx]);

    glUniform1f(glGetUniformLocation(screen_shaders[idx]->ID, "width"), TESettings::VIEWPORT_X);
    glUniform1f(glGetUniformLocation(screen_shaders[idx]->ID, "height"), TESettings::VIEWPORT_Y);
    glUniform2fv(
        glGetUniformLocation(screen_shaders[idx]->ID, "mousePosition"), 1,
        glm::value_ptr(glm::vec2(shader_mouse_position.x / TESettings::SCREEN_X,
                                shader_mouse_position.y / TESettings::SCREEN_Y)));

    set_shader_time(screen_shaders[idx]);

    // GLint width = 0;
    // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    // if (width == 0) std::cout << "Framebuffer Texture not valid!" << std::endl;

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void set_shader_time(std::shared_ptr<Shader> shader) {
    if (!shader)
      return;

    GLint timeLoc = glGetUniformLocation(shader->ID, "time");
    if (timeLoc != -1)
      glUniform1f(timeLoc, glfwGetTime());

    if (screen_shaders.size() > 0) {
      // GLint lightPosLoc =
      //     glGetUniformLocation(screen_shaders[0]->ID, "lightPosition");
      // if (lightPosLoc != -1)
      //   glUniform2fv(lightPosLoc, 1, glm::value_ptr(glm::vec2(0.0f, 0.0f)));

      GLint resLoc = glGetUniformLocation(screen_shaders[0]->ID, "resolution");
      if (resLoc != -1)
        glUniform2fv(resLoc, 1,
                     glm::value_ptr(glm::vec2(TESettings::SCREEN_X,
                                              TESettings::SCREEN_Y)));
    }
  }

  void set_shader_mouse_position(glm::vec2 mouse_pos) {
    shader_mouse_position = mouse_pos;
  }

  static void framebuffer_size_callback(GLFWwindow *window, int width,
                                        int height) {
    TESettings::rescale_window(width, height);
    WindowManager *wm =
        static_cast<WindowManager *>(glfwGetWindowUserPointer(window));
    wm->resize_framebuffers(width, height);
  }
  static void window_size_callback(GLFWwindow *window, int width, int height) {
    TESettings::rescale_window(width, height);
  }
  void mouse_move_handler(MouseMoveEvent *event) {
    mouse_position = event->screen_position;

    mis->mouse_move_handler(*cm, event);
  }
  void mouse_click_handler(MouseEvent *event) {
    mis->mouse_click_handler(*cm, event);
  }
  void mouse_release_handler(MouseEvent *event) {
    mis->mouse_release_handler(*cm, event);
  }
  void mouse_scroll_handler(MouseEvent *event) {
    mis->mouse_scroll_handler(*cm, event);
  }

  static void cursor_position_callback(GLFWwindow *window, double xpos,
                                       double ypos) {
    WindowManager *wm =
        static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

    wm->set_shader_mouse_position(glm::vec2(xpos, ypos));

    // Scale mouse position from screen space to framebuffer (viewport) space
    double scale_x = double(TESettings::VIEWPORT_X) / TESettings::SCREEN_X;
    double scale_y = double(TESettings::VIEWPORT_Y) / TESettings::SCREEN_Y;
    double fb_x = xpos * scale_x;
    double fb_y = ypos * scale_y;

    // Center the coordinates
    fb_x -= TESettings::VIEWPORT_X * 0.5;
    fb_y -= TESettings::VIEWPORT_Y * 0.5;

    // Normalize and map to world coordinates
    double xnorm = fb_x / TESettings::VIEWPORT_X;
    double world_x = xnorm * TESettings::VIEWPORT_X;
    double ynorm = fb_y / TESettings::VIEWPORT_Y;
    double world_y = ynorm * TESettings::VIEWPORT_Y;

    wm->mouse_move_handler(
        new MouseMoveEvent("MouseMove", glm::vec2(world_x, world_y), glm::vec2(xpos, ypos)));
  }
  static void scroll_callback(GLFWwindow *window, double xoffset,
                              double yoffset) {
    WindowManager *wm =
        static_cast<WindowManager *>(glfwGetWindowUserPointer(window));
    wm->mouse_scroll_handler(
        new MouseEvent("MouseScroll", wm->mouse_position, wm->raw_mouse_position, xoffset, yoffset));
  }
  static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                    int mods) {
    WindowManager *wm =
        static_cast<WindowManager *>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    double raw_xpos = xpos;
    double raw_ypos = ypos;

    xpos -= TESettings::SCREEN_X * 0.5;
    ypos -= TESettings::SCREEN_Y * 0.5;

    double xnorm = xpos / TESettings::SCREEN_X;
    double world_x = xnorm * TESettings::VIEWPORT_X;
    double ynorm = ypos / TESettings::SCREEN_Y;
    double world_y = ynorm * TESettings::VIEWPORT_Y;

    xpos = world_x;
    ypos = world_y;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      wm->mouse_click_handler(
          new MouseEvent("LeftMousePress", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      wm->mouse_release_handler(
          new MouseEvent("LeftMouseRelease", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      wm->mouse_click_handler(
          new MouseEvent("RightMousePress", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
      wm->mouse_release_handler(
          new MouseEvent("RightMouseRelease", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
      wm->mouse_click_handler(
          new MouseEvent("MiddleMousePress", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
      wm->mouse_release_handler(
          new MouseEvent("MiddleMouseRelease", glm::vec2(xpos, ypos), glm::vec2(raw_xpos, raw_ypos)));
    }
  }
};
