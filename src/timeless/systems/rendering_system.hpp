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

  float inst_jitter = 0.0f;
  float inst_jitter_speed = 0.0f;

  float ui_jitter = 0.0f;
  float ui_jitter_speed = 0.0f;

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
                                     int tick = 0) {
    if (shader != nullptr && transform != nullptr) {
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                         GL_FALSE, glm::value_ptr(transform->projection));

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

      if (shader != nullptr && transform != nullptr) {
        set_shader_transform_uniforms(shader, transform, tick);
      }

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
      auto animation = cm.get_component<Animation>(entity);
      if (animation != nullptr) {
        for (const auto& bone : animation->bones) {
            if (bone.transform) {
              bone.transform->update_camera(cam->get_position());
              bone.transform->update(x, y, zoom);
              set_shader_transform_uniforms(shader, bone.transform, tick);
            }
            if (bone.sprite) {
                bone.sprite->index = bone.sprite_index;
                set_shader_sprite_uniforms(bone.shader, bone.sprite,
                                          tick, cam->get_position());

                bone.sprite->update();
                bone.quad->render();
                bone.texture->render();
                bone.sprite->render();
            }
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

  void init_instanced_buffers(std::shared_ptr<Quad> quad, std::shared_ptr<Shader> shader, size_t max_instances) {
    quad->render();

    glGenBuffers(1, &instanceVBO);   // Model matrices
    glGenBuffers(1, &instanceVBO2);  // Sprite indices
    glGenBuffers(1, &instanceVBO3);  // Sprite sizes

    // Model Matrix (mat4) as 4 vec4 attributes
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    GLuint attrLoc1 = glGetAttribLocation(shader->ID, "aModel");
    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(attrLoc1 + i);
      glVertexAttribPointer(attrLoc1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
      glVertexAttribDivisor(attrLoc1 + i, 1);
    }

    // Sprite Index (float)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    GLuint attrLoc2 = glGetAttribLocation(shader->ID, "aIndex");
    glEnableVertexAttribArray(attrLoc2);
    glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(attrLoc2, 1);

    // Sprite Size (vec2)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);

    GLuint attrLoc3 = glGetAttribLocation(shader->ID, "aSpriteSize");
    glEnableVertexAttribArray(attrLoc3);
    glVertexAttribPointer(attrLoc3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexAttribDivisor(attrLoc3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // quad->unbind_vao();
  }

  void prepare_instanced(ComponentManager &cm, float x, float y, int zoom)
  {
    models.clear();
    sprite_indices.clear();
    sprite_sizes.clear();
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    for (auto &entity : registered_entities) {
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        transform->update_camera(cam->get_position());
        transform->update(x, y, zoom);
        models.push_back(transform->model);
      }

      auto sprite = cm.get_component<Sprite>(entity);
      if (sprite != nullptr) {
        sprite_indices.push_back(sprite->index);
        sprite_sizes.push_back(sprite->spriteSize);
      }
    }

    // Update buffer data (no buffer creation here!)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    if (!models.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, models.size() * sizeof(glm::mat4), models.data());

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
    if (!sprite_indices.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, sprite_indices.size() * sizeof(float), sprite_indices.data());

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
    if (!sprite_sizes.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, sprite_sizes.size() * sizeof(glm::vec2), sprite_sizes.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void instanced_render(ComponentManager &cm, int x, int y, std::shared_ptr<Quad> quad, std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader, float zoom = 1.0,
              int tick = 0) {
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    shader->use();
    quad->render();
    texture->render();

    glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
    glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1,
                  glm::value_ptr(glm::vec4(1.0f)));
    glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1,
                  glm::value_ptr(glm::vec2(texture->width, texture->height)));

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

    // quad->unbind_vao();
  }
};
