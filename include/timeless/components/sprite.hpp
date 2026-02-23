#pragma once
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <iostream>
#include <queue>
#include <memory>

#include "timeless/components/component.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/timer.hpp"

#include "stb_image.h"

class Sprite : public Component {
private:
  Timer animationSpeed = Timer(0.075f);
  bool isStatic = true;
  int frames;
  int og_index;

  std::queue<float> opacities;

  GLuint framebuffer;
  GLuint tex;
  std::shared_ptr<Quad> slice_quad;
  std::shared_ptr<Texture> slice_texture;

public:
  bool animating;
  bool flip = false;
  bool hidden = false;

  glm::vec4 color;
  glm::vec4 og_color;
  int index;
  glm::vec2 spriteSheetSize;
  glm::vec2 spriteSize;

  std::shared_ptr<Shader> slice_shader;

  // Rule of 5
  Sprite(const Sprite&) = delete;
  Sprite& operator=(const Sprite&) = delete;

  Sprite(Sprite &&other) noexcept
      : animationSpeed(std::move(other.animationSpeed)),
        isStatic(other.isStatic), frames(other.frames),
        og_index(other.og_index), opacities(std::move(other.opacities)),
        framebuffer(other.framebuffer), tex(other.tex),
        slice_quad(std::move(other.slice_quad)),
        slice_texture(std::move(other.slice_texture)),
        animating(other.animating), flip(other.flip), hidden(other.hidden),
        color(other.color), og_color(other.og_color), index(other.index),
        spriteSheetSize(other.spriteSheetSize), spriteSize(other.spriteSize),
        slice_shader(std::move(other.slice_shader)) {
    other.framebuffer = 0;
    other.tex = 0;
    other.slice_quad = nullptr;
    other.slice_texture = nullptr;
    other.slice_shader = nullptr;
  }

  Sprite &operator=(Sprite &&other) noexcept {
    if (this != &other) {
      // Clean up current resources
      if (slice_shader) {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &tex);
      }
      animationSpeed = std::move(other.animationSpeed);
      isStatic = other.isStatic;
      frames = other.frames;
      og_index = other.og_index;
      opacities = std::move(other.opacities);
      framebuffer = other.framebuffer;
      tex = other.tex;
      slice_quad = std::move(other.slice_quad);
      slice_texture = std::move(other.slice_texture);
      animating = other.animating;
      flip = other.flip;
      hidden = other.hidden;
      color = other.color;
      og_color = other.og_color;
      index = other.index;
      spriteSheetSize = other.spriteSheetSize;
      spriteSize = other.spriteSize;
      slice_shader = std::move(other.slice_shader);

      other.framebuffer = 0;
      other.tex = 0;
      other.slice_quad = nullptr;
      other.slice_texture = nullptr;
      other.slice_shader = nullptr;
    }
    return *this;
  }

  Sprite(int index, glm::vec4 color, glm::vec2 spriteSheetSize,
         glm::vec2 spriteSize, bool isStatic = true, bool animating = false,
         int frames = 6);
  ~Sprite();
  void setColor(glm::vec4 newColor); 
  void set_index(int newIndex);
  void set_slice_shader(std::shared_ptr<Shader> shader,
                        std::shared_ptr<Quad> quad,
                        std::shared_ptr<Texture> texture);

  void set_shader_sprite_uniforms(int textureUnit = 0);

  void set_shader_transform_uniforms(int tick = 0);
  void render_sliced_to_texture();

  void render_sliced();

  void set_opacity_frames(float from, float to, float speed = 1.0);

  void append_opacity_frames(float from, float to, float speed = 1.0);
  void update();
  void render();
};
