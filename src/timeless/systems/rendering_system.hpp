#pragma once
#include "../components/shader.hpp"
#include "../components/transform.hpp"
#include "../components/animation.hpp"
#include "system.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/timeless.hpp"
#include <string.h>

class RenderingSystem : public System {
private:
  unsigned int instanceVBO;
  unsigned int instanceVBO2;
  unsigned int instanceVBO3;

  unsigned int attrLoc1;
  unsigned int attrLoc2;
  unsigned int attrLoc3;

  std::vector<glm::mat4> models;
  std::vector<float> sprite_indices;
  std::vector<glm::vec2> sprite_sizes;

public:
  Entity camera;

  float inst_jitter = 0.15;
  float inst_jitter_speed = 1.0;

  float ui_jitter = 0.005;
  float ui_jitter_speed = 6.0;

  void purge(ComponentManager &cm) {
    for(auto &ent : registered_entities) {
      cm.remove_entity(ent);
    }
    registered_entities.clear();
    models.clear();
    sprite_indices.clear();
    sprite_sizes.clear();
  }

  void register_camera(Entity c) { camera = c; }

  void set_shader_transform_uniforms(std::shared_ptr<Shader> shader,
                                     std::shared_ptr<Transform> transform,
                                     std::shared_ptr<Animation> animation = nullptr, 
                                     int tick = 0) {
    if (shader != nullptr && transform != nullptr) {
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                         GL_FALSE, glm::value_ptr(transform->projection));

      if(animation != nullptr) {
        if(!animation->positions.empty()) {
          glm::vec3 dir = animation->positions.front();

          transform->model = glm::mat4(1.0f);
          glm::mat4 anim_mat = glm::translate(glm::mat4(1.0f), dir - transform->offset);

          transform->model = glm::translate(transform->model, glm::vec3(0.5 * transform->width, 0.0, 0.0));
          transform->model = anim_mat * transform->model;
          transform->model = glm::scale(transform->model, glm::vec3(transform->width, transform->height, 1.0));

          if(animation->scales.empty()) {
            transform->model = glm::scale(transform->model, transform->scale);
          }

          animation->positions.pop();
          if (animation->loop) {
            animation->positions.push(dir);
          }
        }

        if(!animation->rotations.empty()) {
          glm::vec3 rot = animation->rotations.front();
          auto eulers = glm::quat(rot);
          glm::mat4 anim_rot = glm::toMat4(eulers);
          transform->model = transform->model * anim_rot;
          animation->rotations.pop();
          if (animation->loop) {
            animation->rotations.push(rot);
          }
        }

        if(!animation->scales.empty()) {
          glm::vec3 scale = animation->scales.front();
          transform->model = glm::scale(transform->model, scale);
          animation->scales.pop();
          if(animation->scales.empty() && !animation->reset) {
            transform->scale = scale;
          }
          if (animation->loop) {
            animation->scales.push(scale);
          }
        }
      }
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                        glm::value_ptr(transform->model));

      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                         glm::value_ptr(transform->view));
      glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
      glUniform1f(glGetUniformLocation(shader->ID, "jitter"), ui_jitter);
      glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"), ui_jitter_speed);
    }
  }
  void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                  std::shared_ptr<Sprite> sprite, int tick = 0,
                                  glm::vec3 cam_position = glm::vec3(0.0)) {
    if (shader != nullptr && sprite != nullptr) {
      float cols = sprite->spriteSheetSize.x / sprite->spriteSize.x;

      float col =  sprite->index - cols * floor(sprite->index/cols);
      float row = floor(sprite->index / cols);

      glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
      glUniform1f(glGetUniformLocation(shader->ID, "index"), static_cast<float>(sprite->index));
      glUniform1f(glGetUniformLocation(shader->ID, "col"), col);
      glUniform1f(glGetUniformLocation(shader->ID, "row"), row);

      glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1,
                   glm::value_ptr(sprite->color));
      glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1,
                   glm::value_ptr(sprite->spriteSheetSize));
      glUniform2fv(glGetUniformLocation(shader->ID, "spriteSize"), 1,
                   glm::value_ptr(sprite->spriteSize));
      glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
      glUniform1f(glGetUniformLocation(shader->ID, "jitter"), ui_jitter);
      glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"), ui_jitter_speed);
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
    }
  }

  void bind_text_values(Entity entity, ComponentManager &cm) {
    auto font = cm.get_component<Font>(entity);
    if (font != nullptr) {
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(font->VAO);
    }
  }

  float get_text_width(Entity entity, ComponentManager &cm) {
    auto font = cm.get_component<Font>(entity);
    auto text = cm.get_component<Text>(entity);

    float width = 0.0f;
    // Get width of string
    std::string::const_iterator c;
    for (c = text->text.begin(); c != text->text.end(); c++) {
      Glyph glyph = font->glyphs.at(*c);
      width += glyph.advance >> 6;
    }
    return width;
  }

  float get_text_height(Entity entity, ComponentManager &cm) {
    auto font = cm.get_component<Font>(entity);
    auto text = cm.get_component<Text>(entity);

    float height = 0.0f;
    // Get width of string
    std::string::const_iterator c;
    for (c = text->text.begin(); c != text->text.end(); c++) {
      Glyph glyph = font->glyphs.at(*c);
      height = glyph.size.y;
    }
    return height;
  }

  void sort(ComponentManager &cm) {

    std::stable_sort(registered_entities.begin(), registered_entities.end(),
                     [&](const Entity &a, const Entity &b) {
                       auto trans_a = cm.get_component<Transform>(a);
                       auto trans_b = cm.get_component<Transform>(b);
                       if (trans_a == nullptr || trans_b == nullptr)
                         return false;
                       return trans_a->position.z < trans_b->position.z;
                     });
  }

  void shadow_rotate(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
      auto transform = cm.get_component<Transform>(entity);
      auto sprite = cm.get_component<Sprite>(entity);
      transform->rotate(glm::radians(glm::vec3(15,3,45)));
      transform->position.x += 16.0;
      transform->position.y -= 8.0;
      transform->scale.y += 12.0;
      sprite->color = glm::vec4(0.1, 0.1, 0.1, 0.4);
    }
  }

  void shadow_reset_rotation(ComponentManager &cm) {
    for (auto &entity : registered_entities) {
      auto transform = cm.get_component<Transform>(entity);
      auto sprite = cm.get_component<Sprite>(entity);
      transform->rotate(glm::radians(glm::vec3(0,0,0)));
      transform->position.x -= 16.0;
      transform->position.y += 8.0;
      transform->scale.y -= 12.0;
      sprite->color = sprite->og_color;
    }
  }

  void render(ComponentManager &cm, int x, int y, float zoom = 1.0,
              int tick = 0) {
    // get camera if set
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    for (auto &entity : registered_entities) {
      auto shader = cm.get_component<Shader>(entity);
      shader->use();
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        if (cam != nullptr) {
          transform->update_camera(cam->get_position());
        }
        transform->update(x, y, zoom);
      }

      auto text = cm.get_component<Text>(entity);
      if (text != nullptr) {
        if (text->center) {
          transform->update(
              x, y, 1.0f,
              glm::vec3(-get_text_width(entity, cm) * 0.5f,
                        -get_text_height(entity, cm) * 0.5f, 0.0f));
        }
        bind_text_values(entity, cm);
      }

      auto animation = cm.get_component<Animation>(entity);
      if (shader != nullptr && transform != nullptr)
        set_shader_transform_uniforms(shader, transform, animation, tick);

      auto quad = cm.get_component<Quad>(entity);
      if (quad != nullptr)
        quad->render();
      auto texture = cm.get_component<Texture>(entity);
      if (texture != nullptr)
        texture->render();
      auto sprite = cm.get_component<Sprite>(entity);
      if (sprite != nullptr) {
        if (!sprite->hidden) {
          set_shader_sprite_uniforms(shader, sprite,
                                    tick, cam->get_position());

          sprite->update();
          sprite->render();
        }
      }

      if (text != nullptr) {
        auto font = cm.get_component<Font>(entity);
        if (!text->hidden) {
          text->render(*font, 0.0f, 0.0f, get_text_height(entity, cm), shader);
        }
      }
    }
  }

  void prepare_instanced(ComponentManager &cm, std::shared_ptr<Quad> quad, std::shared_ptr<Shader> shader, float x, float y, int zoom)
  {
    models.clear();
    sprite_indices.clear();
    sprite_sizes.clear();
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    if(registered_entities.size() > 0) {
      for (auto &entity : registered_entities) {
        auto transform = cm.get_component<Transform>(entity);
        if(transform != nullptr){
          transform->update_camera(cam->get_position());
          transform->update(x, y, zoom);
          models.push_back(transform->model);
        }
        
        auto sprite = cm.get_component<Sprite>(entity);
        if(sprite != nullptr){
          sprite_indices.push_back(sprite->index);
          sprite_sizes.push_back(sprite->spriteSize);
        }
      }

      int mat4size = sizeof(glm::mat4);
      int floatsize = sizeof(float);
      int vec2size = sizeof(glm::vec2);

      shader->use();
      glGenBuffers(1, &instanceVBO);
      glGenBuffers(1, &instanceVBO2);
      glGenBuffers(1, &instanceVBO3);

      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2); // this attribute comes from a different vertex buffer
      glBufferData(GL_ARRAY_BUFFER, floatsize * registered_entities.size(), &sprite_indices[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3); // this attribute comes from a different vertex buffer
      glBufferData(GL_ARRAY_BUFFER, vec2size * registered_entities.size(), &sprite_sizes[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
      glBufferData(GL_ARRAY_BUFFER, mat4size * registered_entities.size(), &models[0], GL_STATIC_DRAW);
      quad->render();

      attrLoc1 = glGetAttribLocation(shader->ID, "aModel");
      attrLoc2 = glGetAttribLocation(shader->ID, "aIndex");
      attrLoc3 = glGetAttribLocation(shader->ID, "aSpriteSize");

      glEnableVertexAttribArray(attrLoc1);
      glVertexAttribPointer(attrLoc1, 4, GL_FLOAT, GL_FALSE, 64, (void*)0);
      glEnableVertexAttribArray(attrLoc1 + 1);
      glVertexAttribPointer(attrLoc1 + 1, 4, GL_FLOAT, GL_FALSE, 64, (void*)(16));
      glEnableVertexAttribArray(attrLoc1 + 2);
      glVertexAttribPointer(attrLoc1 + 2, 4, GL_FLOAT, GL_FALSE, 64, (void*)(2 * 16));
      glEnableVertexAttribArray(attrLoc1 + 3);
      glVertexAttribPointer(attrLoc1 + 3, 4, GL_FLOAT, GL_FALSE, 64, (void*)(3 * 16));
      //
      glVertexAttribDivisor(attrLoc1, 1); // tell OpenGL this is an instanced vertex attribute.
      glVertexAttribDivisor(attrLoc1 + 1, 1); // tell OpenGL this is an instanced vertex attribute.
      glVertexAttribDivisor(attrLoc1 + 2, 1); // tell OpenGL this is an instanced vertex attribute.
      glVertexAttribDivisor(attrLoc1 + 3, 1); // tell OpenGL this is an instanced vertex attribute.

      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2); // this attribute comes from a different vertex buffer
      glEnableVertexAttribArray(attrLoc2);
      glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, floatsize, (void*)0);
      glVertexAttribDivisor(attrLoc2, 1); // tell OpenGL this is an instanced vertex attribute.
      //
      glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3); // this attribute comes from a different vertex buffer
      glEnableVertexAttribArray(attrLoc3);
      glVertexAttribPointer(attrLoc3, 2, GL_FLOAT, GL_FALSE, vec2size, (void*)0);
      glVertexAttribDivisor(attrLoc3, 1); // tell OpenGL this is an instanced vertex attribute.
    }
  }
  void instanced_render(ComponentManager &cm, int x, int y, std::shared_ptr<Quad> quad, std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader, float zoom = 1.0,
              int tick = 0) {
    // get camera if set
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    float t_width = 256.0f;
    float t_height = 128.0f;

    shader->use();
    quad->render();
    texture->render();

    glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
    glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1,
                  glm::value_ptr(glm::vec4(1.0f)));
    glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1,
                  glm::value_ptr(glm::vec2(texture->width, texture->height)));

    // glm::mat4 transformMatrix = glm::mat4(1.0f);
    // transformMatrix = glm::translate(transformMatrix, cam->get_position());

    glm::mat4 view = glm::lookAt(cam->get_position(), cam->get_position() + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::ortho(-(static_cast<float>(x * 0.5) * zoom), static_cast<float>(x * 0.5) * zoom, (static_cast<float>(y * 0.5) * zoom), -static_cast<float>(y * 0.5) * zoom, -1000.0f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                        GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                        glm::value_ptr(view));
    glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
    glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
    glUniform1f(glGetUniformLocation(shader->ID, "jitter"), inst_jitter);
    glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"), inst_jitter_speed);

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, registered_entities.size());
  }
};
