#pragma once

#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

#include "timeless/components/camera.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/systems/system.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>

// Immediate-mode debug drawing. Each frame:
//   1. Call draw_line / draw_rect from anywhere.
//   2. Call flush() once, e.g. after RenderingSystem::render().
// The vertex buffer is cleared after every flush.
class DebugSystem : public System {
private:
  std::vector<float> vertices; // interleaved: x y z r g b a
  unsigned int VAO = 0, VBO = 0;
  std::shared_ptr<Shader> shader;
  Entity camera_entity = 0;

  void push_vertex(glm::vec3 p, glm::vec4 c);

public:
  DebugSystem();
  ~DebugSystem();

  void register_camera(Entity e);
  void draw_line(glm::vec3 a, glm::vec3 b,
                 glm::vec4 color = glm::vec4(1, 0, 1, 1));
  void draw_rect(glm::vec3 center, float hw, float hh,
                 glm::vec4 color = glm::vec4(1, 0, 1, 1));
  void render(ComponentManager &cm, int vp_x, int vp_y, float zoom = 1.0f);
};
