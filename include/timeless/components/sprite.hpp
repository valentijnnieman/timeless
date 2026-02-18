#pragma once

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <iostream>

#include "timeless/components/component.hpp"
#include "timeless/components/shader.hpp"
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

  Sprite(Sprite&& other) noexcept
      : animationSpeed(std::move(other.animationSpeed)),
        isStatic(other.isStatic),
        frames(other.frames),
        og_index(other.og_index),
        opacities(std::move(other.opacities)),
        framebuffer(other.framebuffer),
        tex(other.tex),
        slice_quad(std::move(other.slice_quad)),
        slice_texture(std::move(other.slice_texture)),
        animating(other.animating),
        flip(other.flip),
        hidden(other.hidden),
        color(other.color),
        og_color(other.og_color),
        index(other.index),
        spriteSheetSize(other.spriteSheetSize),
        spriteSize(other.spriteSize),
        slice_shader(std::move(other.slice_shader))
  {
      other.framebuffer = 0;
      other.tex = 0;
      other.slice_quad = nullptr;
      other.slice_texture = nullptr;
      other.slice_shader = nullptr;
  }

  Sprite& operator=(Sprite&& other) noexcept {
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
         int frames = 6)
      : color(color), og_color(color), index(index), og_index(index),
        isStatic(isStatic), animating(animating), frames(frames),
        spriteSheetSize(spriteSheetSize), spriteSize(spriteSize) {}
  ~Sprite() {
    if (slice_shader) {
      glDeleteFramebuffers(1, &framebuffer);
      glDeleteTextures(1, &tex);
    }
  }
  void setColor(glm::vec4 newColor) { color = newColor; }
  void set_index(int newIndex) {
    index = newIndex;
    og_index = newIndex;
  }
  void set_slice_shader(std::shared_ptr<Shader> shader,
                        std::shared_ptr<Quad> quad,
                        std::shared_ptr<Texture> texture) {
    slice_quad = quad;
    slice_texture = texture;
    slice_shader = shader;

    if(spriteSize.x == 0 || spriteSize.y == 0) {
      std::cerr << "Sprite size is zero, cannot create framebuffer!" << std::endl;
      return;
    }
    // Create a texture for the framebuffer (destination)
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spriteSize.x, spriteSize.y, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Set texture parameters (important!)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 3. Attach the texture to the framebuffer as color attachment 0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "FBO incomplete!" << std::endl;
    }

    // GLint width = 0;
    // glBindTexture(GL_TEXTURE_2D, tex);
    // glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    // if (width == 0) std::cerr << "Sprite framebuffer texture not valid!" << std::endl;

    // 4. Render the sliced sprite to the framebuffer
    render_sliced_to_texture();

    // 5. Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore active texture unit to 0 (default)
    glActiveTexture(GL_TEXTURE0);
  }

  void set_shader_sprite_uniforms(int textureUnit = 0) {
    float cols = spriteSheetSize.x / spriteSize.x;

    float col = index - cols * floor(index / cols);
    float row = floor(index / cols);

    glUniform1i(glGetUniformLocation(slice_shader->ID, "texture1"),
                textureUnit);
    glUniform1f(glGetUniformLocation(slice_shader->ID, "index"),
                static_cast<float>(index));
    glUniform1f(glGetUniformLocation(slice_shader->ID, "col"), col);
    glUniform1f(glGetUniformLocation(slice_shader->ID, "row"), row);

    glUniform4fv(glGetUniformLocation(slice_shader->ID, "highlightColor"), 1,
                 glm::value_ptr(color));
    glUniform2fv(glGetUniformLocation(slice_shader->ID, "spriteSheetSize"), 1,
                 glm::value_ptr(spriteSheetSize));
    glUniform2fv(glGetUniformLocation(slice_shader->ID, "spriteSize"), 1,
                 glm::value_ptr(spriteSize));
    glUniform1f(glGetUniformLocation(slice_shader->ID, "time"),
                static_cast<float>(glfwGetTime()));
    // glUniform1f(glGetUniformLocation(shader->ID, "jitter"),
    // ui_jitter); glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"),
    //             ui_jitter_speed);
    // glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
  }

  void set_shader_transform_uniforms(int tick = 0) {
    glUniformMatrix4fv(glGetUniformLocation(slice_shader->ID, "projection"), 1,
                       GL_FALSE,
                       glm::value_ptr(glm::ortho(0.0f, spriteSize.x * 2.0f,
                                                 0.0f, spriteSize.y * 2.0f)));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(spriteSize.x, spriteSize.y, 0.0));
    model = glm::scale(model, glm::vec3(spriteSize.x, spriteSize.y, 1.0));
    glUniformMatrix4fv(glGetUniformLocation(slice_shader->ID, "model"), 1,
                       GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(slice_shader->ID, "view"), 1,
                       GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
  }

  void render_sliced_to_texture() {
    // 1. Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, spriteSize.x, spriteSize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClearColor(TESettings::SCREEN_COLOR.r, TESettings::SCREEN_COLOR.g,
    //               TESettings::SCREEN_COLOR.b, TESettings::SCREEN_COLOR.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render your sprite here using slice_shader
    slice_shader->use();
    slice_quad->render();
    slice_texture->render();

    set_shader_sprite_uniforms();
    set_shader_transform_uniforms();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // 4. Unbind the framebuffer to return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, TESettings::VIEWPORT_X, TESettings::VIEWPORT_Y);
  }

  void render_sliced() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void set_opacity_frames(float from, float to, float speed = 1.0) {
    while (!opacities.empty())
      opacities.pop();

    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from, to, i);
      opacities.push(x_l);
    }
  }

  void append_opacity_frames(float from, float to, float speed = 1.0) {
    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from, to, i);
      opacities.push(x_l);
    }
  }
  void update() {
    if (animationSpeed.pollTime() && animating) {
      index += 1;
      if (index >= (og_index + frames)) {
        index = og_index;
      }
    } else if (!animating && !isStatic) {
      index = og_index;
    }

    if (!opacities.empty()) {
      float o = opacities.front();
      color.a = o;
      opacities.pop();
    }
  }
  void render() { 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

    // GLenum err;
    // while ((err = glGetError()) != GL_NO_ERROR) {
    //     std::cerr << "OpenGL error in sprite.render(): " << err << std::endl;
    // }
  }
};
