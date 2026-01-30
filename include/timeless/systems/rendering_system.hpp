#pragma once
#include "timeless/components/animation.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/systems/system.hpp"
#include "timeless/timeless.hpp"
#include <memory>
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

  Entity debug_ligth_ent;

  float inst_jitter = 0.0f;
  float inst_jitter_speed = 0.0f;

  float ui_jitter = 0.0f;
  float ui_jitter_speed = 0.0f;

  void purge(ComponentManager &cm) {
    for (auto &ent : registered_entities) {
      cm.remove_entity(ent);
    }
    registered_entities.clear();
    models.clear();
    sprite_indices.clear();
    sprite_sizes.clear();
  }

  void register_camera(Entity c) { 
    camera = c; 
  }

  void update_transform(std::shared_ptr<Transform> transform, glm::vec3 o = glm::vec3(0.0f))
  {
      transform->model = glm::mat4(1.0f);

      transform->model = glm::translate(transform->model, transform->get_position_minus_offset());

      if(!transform->isometric){
        if (transform->center) {
          transform->model =
              glm::translate(transform->model, glm::vec3(0.5 * transform->width, 0.5 * transform->height, 0.0));
        } else {
          if(transform->width != 0.0f && transform->height != 0.0f){
            transform->model = glm::translate(transform->model, glm::vec3(0.5 * transform->width, 0.0, 0.0));
          }
        }
      }
      if(o != glm::vec3(0.0f))
        transform->model = glm::translate(transform->model, o);

      if (transform->flip) {
        transform->model = glm::rotate(transform->model, glm::radians(180.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
      }

      if(transform->isometric){
        // model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1, 0, 0));
        // model = glm::rotate(model, glm::radians(63.559f), glm::vec3(0, 0, 1));
        transform->model = glm::rotate(transform->model, glm::radians(45.0f), glm::vec3(0, 0, 1));
      }

      glm::mat4 rotation_matrix = transform->getRotationMatrix();

      transform->model = transform->model * rotation_matrix;

      if(transform->width != 0.0f && transform->height != 0.0f){
        transform->model = glm::scale(transform->model, glm::vec3(transform->width, transform->height, transform->width));
      }
      glm::vec3 s = transform->get_scale();
      transform->model = glm::scale(transform->model, s);
    }

  void set_shader_transform_uniforms(std::shared_ptr<Shader> shader,
                                     std::shared_ptr<Transform> transform,
                                     std::shared_ptr<Camera> camera,
                                     int x, int y, float zoom,
                                     int tick = 0) {
    if (shader != nullptr && transform != nullptr) {
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                         GL_FALSE, glm::value_ptr(camera->get_projection_matrix(x, y, zoom)));
        
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                         glm::value_ptr(transform->model));

      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                         glm::value_ptr(camera->get_view_matrix()));
        
      glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
      // glUniform1f(glGetUniformLocation(shader->ID, "jitter"), ui_jitter);
      // glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"),
      //             ui_jitter_speed);
    }
  }
  void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                  std::shared_ptr<Sprite> sprite, int tick = 0,
                                  glm::vec3 cam_position = glm::vec3(0.0)) {
    if (shader != nullptr && sprite != nullptr) {
      float cols = sprite->spriteSheetSize.x / sprite->spriteSize.x;

      float col = sprite->index - cols * floor(sprite->index / cols);
      float row = floor(sprite->index / cols);

      glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
      glUniform1f(glGetUniformLocation(shader->ID, "index"),
                  static_cast<float>(sprite->index));
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
      glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"),
                  ui_jitter_speed);
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
      // glUniform1f(glGetUniformLocation(shader->ID, "cutoffDistance"),
      // 100.0f);
    }
  }

  float get_text_width(Entity entity, ComponentManager &cm) {
    auto font = cm.get_component<Font>(entity);
    auto text = cm.get_component<Text>(entity);

    float width = 0.0f;
    // Get width of string
    std::string::const_iterator c;
    for (c = text->text.begin(); c != text->text.end(); c++) {
      auto glyphIt = font->glyphs.find(*c);
      // if (glyphIt == font->glyphs.end()) {
      //   // Optionally handle/log missing glyph
      //   continue;
      // }
      Glyph glyph = glyphIt->second;
      width += glyph.advance >> 6;
    }
    return width;
  }

  float get_text_height(Entity entity, ComponentManager &cm) {
    auto font = cm.get_component<Font>(entity);
    auto text = cm.get_component<Text>(entity);

    float height = 0.0f;
    // Get height of string
    std::string::const_iterator c;
    for (c = text->text.begin(); c != text->text.end(); c++) {
      auto glyphIt = font->glyphs.find(*c);
      // if (glyphIt == font->glyphs.end()) {
      //   // Optionally handle/log missing glyph
      //   continue;
      // }
      Glyph glyph = glyphIt->second;
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
      transform->setRotationEuler(glm::vec3(15, 3, 45));
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
      transform->setRotationEuler(glm::vec3(0, 0, 0));
      transform->position.x -= 16.0;
      transform->position.y += 8.0;
      transform->scale.y -= 12.0;
      sprite->color = sprite->og_color;
    }
  }

  void calculate_sun(ComponentManager &cm, std::shared_ptr<Shader> shader,
                     std::shared_ptr<Camera> cam, int tick) {
    // Angle goes from -π/2 (sunrise) to π/2 (sunset)
    float angle = glm::mix(
        -glm::half_pi<float>(), glm::half_pi<float>(),
        (float)tick / (TESettings::MAX_TICKS / 2.0)); // dayProgress: 0.0 to 1.0
    //
    glm::vec3 lightPos;
    lightPos.x = 1.0f;
    lightPos.y = 0.0f;
    lightPos.z = 1.0f;

    // auto light_transfrom = cm.get_component<Transform>(debug_ligth_ent);
    // if (light_transfrom != nullptr) {
    //   light_transfrom->position = lightPos * 100.0f;
    // }

    // glUniform1f(glGetUniformLocation(shader->ID, "ambientStrength"),
    // sin(angle) * 0.5f + 0.6f);
    glUniform1f(glGetUniformLocation(shader->ID, "ambientStrength"), 0.3f);

    glUniform3fv(glGetUniformLocation(shader->ID, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shader->ID, "cameraPos"), 1,
                 glm::value_ptr(cam->get_position()));
  }

  void render(ComponentManager &cm, int x, int y, float zoom = 1.0,
              int tick = 0, float delta_time = 0.016f) {
    // get camera if set
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    for (auto &entity : registered_entities) {
      auto sprite = cm.get_component<Sprite>(entity);
      auto shader = cm.get_component<Shader>(entity);
      if(shader != nullptr) {
        shader->use();
      }
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        if (cam != nullptr) {
          transform->update_camera(cam);
        }
        update_transform(transform);
      }

      auto animation = cm.get_component<Animation>(entity);
      if (animation != nullptr && animation->playing) {
        animation->root->transform->update_camera(cam);
        update_transform(animation->root->transform);
        auto root_shader = cm.get_component<Shader>(animation->root->entity);
        set_shader_transform_uniforms(root_shader, animation->root->transform,
                                      cam, x, y, zoom, tick);

        if (auto rootModelBone =
                std::dynamic_pointer_cast<ModelBone>(animation->root)) {
          auto model = rootModelBone->model;
          if (model != nullptr) {
            calculate_sun(cm, root_shader, cam, tick);
            auto texture = cm.get_component<Texture>(entity);
            if(texture != nullptr) {
              texture->render();
            }
            model->render(rootModelBone->transform->model, delta_time);
          }
        }

        if (auto rootSpriteBone =
                std::dynamic_pointer_cast<SpriteBone>(animation->root)) {
          rootSpriteBone->sprite->index = rootSpriteBone->sprite_index;
          rootSpriteBone->sprite->update();
          rootSpriteBone->quad->render();

          if (rootSpriteBone->sprite->slice_shader != nullptr) {
            set_shader_sprite_uniforms(root_shader, rootSpriteBone->sprite,
                                       tick, cam->get_position());
            rootSpriteBone->sprite->render_sliced();
          } else {
            if (rootSpriteBone->texture != nullptr) {
              rootSpriteBone->texture->render();
              set_shader_sprite_uniforms(root_shader, rootSpriteBone->sprite,
                                         tick, cam->get_position());
            }
            rootSpriteBone->sprite->render();
          }
        }
        for (const auto &bone : animation->bones) {
          auto bone_shader = cm.get_component<Shader>(bone->entity);
          if (bone_shader)
            bone_shader->use();
          if (bone->transform) {
            bone->transform->update_camera(cam);
            update_transform(bone->transform);
            set_shader_transform_uniforms(bone_shader, bone->transform, cam, x,
                                          y, zoom, tick);
          }

          if (auto spriteBone = std::dynamic_pointer_cast<SpriteBone>(bone)) {
            if (spriteBone->sprite) {
              spriteBone->sprite->index = spriteBone->sprite_index;

              spriteBone->sprite->update();
              spriteBone->quad->render();
              if (spriteBone->sprite->slice_shader != nullptr) {
                spriteBone->sprite->render_sliced();
              } else {
                // if (animation->root.texture != nullptr)
                spriteBone->texture->render();
                set_shader_sprite_uniforms(bone_shader, spriteBone->sprite, tick,
                                           cam->get_position());
                spriteBone->sprite->render();
              }
            }
          }
        }
      }

      if (animation == nullptr || !animation->playing) {
        auto text = cm.get_component<Text>(entity);
        if (text != nullptr) {
          if (text->center) {
            update_transform(transform, glm::vec3(-get_text_width(entity, cm) * 0.5f,
                                        -get_text_height(entity, cm) * 0.5f,
                                        0.0f));
          }
        }
        if(text != nullptr) {
          auto font = cm.get_component<Font>(entity);
          if(font != nullptr) {
            if (!text->hidden) {
              set_shader_transform_uniforms(shader, transform, cam, x, y, zoom, tick);
              text->render(font, 0.0f, 0.0f, get_text_height(entity, cm),
                          shader);
            }
          }
        }

        auto quad = cm.get_component<Quad>(entity);
        if (quad != nullptr) {
          quad->render();
        }
        if (sprite != nullptr) {
          if (!sprite->hidden) {
            sprite->update();

            if (sprite != nullptr && sprite->slice_shader != nullptr) {
              sprite->render_sliced();
            } else {
              auto texture = cm.get_component<Texture>(entity);
              if (texture != nullptr) {

                set_shader_transform_uniforms(shader, transform, cam, x, y, zoom, tick);
                set_shader_sprite_uniforms(shader, sprite, tick,
                                          cam->get_position());
                texture->render();
                sprite->render();
              } else {
                std::cout << "No texture for sprite entity " << entity << std::endl;
              }
            }
          }
        }

        auto model = cm.get_component<Model>(entity);
        if (model != nullptr) {
          BoundingSphere sphere = transform->get_bounding_sphere();
          if (is_in_frustum(cam->get_frustum(x, y, zoom), sphere)) {
            // update_transform(transform);
            set_shader_transform_uniforms(shader, transform, cam, x, y, zoom, tick);
            calculate_sun(cm, shader, cam, tick);
            auto texture = cm.get_component<Texture>(entity);
            if(texture != nullptr) {
              texture->render();
            }
            model->render(transform->model, delta_time);
          }
        }
      }
    }
  }

  bool is_in_frustum(const Frustum& frustum, const BoundingSphere& sphere) {
      for (int i = 0; i < 6; ++i) {
          const glm::vec4& plane = frustum.planes[i];
          // Plane equation: ax + by + cz + d = 0
          float distance = glm::dot(glm::vec3(plane), sphere.center) + plane.w;
          if (distance < -(sphere.radius * 6.0f)) {
              // Sphere is completely outside this plane
              return false;
          }
      }
      // Sphere is inside or intersects all planes
      return true;
  }

  void init_instanced_buffers(std::shared_ptr<Quad> quad,
                              std::shared_ptr<Shader> shader,
                              size_t max_instances) {
    quad->render();

    glGenBuffers(1, &instanceVBO);  // Model matrices
    glGenBuffers(1, &instanceVBO2); // Sprite indices
    glGenBuffers(1, &instanceVBO3); // Sprite sizes

    // Model Matrix (mat4) as 4 vec4 attributes
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::mat4), nullptr,
                 GL_DYNAMIC_DRAW);

    GLuint attrLoc1 = glGetAttribLocation(shader->ID, "aModel");
    for (int i = 0; i < 4; ++i) {
      glEnableVertexAttribArray(attrLoc1 + i);
      glVertexAttribPointer(attrLoc1 + i, 4, GL_FLOAT, GL_FALSE,
                            sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
      glVertexAttribDivisor(attrLoc1 + i, 1);
    }

    // Sprite Index (float)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);

    GLuint attrLoc2 = glGetAttribLocation(shader->ID, "aIndex");
    glEnableVertexAttribArray(attrLoc2);
    glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                          (void *)0);
    glVertexAttribDivisor(attrLoc2, 1);

    // Sprite Size (vec2)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::vec2), nullptr,
                 GL_DYNAMIC_DRAW);

    GLuint attrLoc3 = glGetAttribLocation(shader->ID, "aSpriteSize");
    glEnableVertexAttribArray(attrLoc3);
    glVertexAttribPointer(attrLoc3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
                          (void *)0);
    glVertexAttribDivisor(attrLoc3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // quad->unbind_vao();
  }

  void prepare_instanced(ComponentManager &cm, float x, float y, int zoom) {
    models.clear();
    sprite_indices.clear();
    sprite_sizes.clear();
    std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

    for (auto &entity : registered_entities) {
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        transform->update_camera(cam);
        update_transform(transform);
        models.push_back(transform->model);
      }

      auto sprite = cm.get_component<Sprite>(entity);
      if (sprite != nullptr) {
        sprite_indices.push_back(sprite->index);
        sprite_sizes.push_back(sprite->spriteSize);
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    if (!models.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, models.size() * sizeof(glm::mat4),
                      models.data());

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
    if (!sprite_indices.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, sprite_indices.size() * sizeof(float),
                      sprite_indices.data());

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
    if (!sprite_sizes.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0,
                      sprite_sizes.size() * sizeof(glm::vec2),
                      sprite_sizes.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void instanced_render(ComponentManager &cm, int x, int y,
                        std::shared_ptr<Quad> quad,
                        std::shared_ptr<Texture> texture,
                        std::shared_ptr<Shader> shader, float zoom = 1.0,
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

    glm::mat4 view = glm::lookAt(cam->get_position(),
                                 cam->get_position() + glm::vec3(0, 0, -1),
                                 glm::vec3(0, 1, 0));
    glm::mat4 projection =
        glm::ortho(-(static_cast<float>(x * 0.5) * zoom),
                   static_cast<float>(x * 0.5) * zoom,
                   (static_cast<float>(y * 0.5) * zoom),
                   -static_cast<float>(y * 0.5) * zoom, -1000.0f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                       GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                       glm::value_ptr(view));
    glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
    glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
    glUniform1f(glGetUniformLocation(shader->ID, "jitter"), inst_jitter);
    glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"),
                inst_jitter_speed);

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                            registered_entities.size());

    // quad->unbind_vao();
  }
};
