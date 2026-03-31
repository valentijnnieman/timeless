#include "timeless/systems/rendering_system.hpp"

void RenderingSystem::purge(ComponentManager &cm) {
  for (auto &ent : registered_entities)
    cm.remove_entity(ent);
  registered_entities.clear();
  models.clear();
  sprite_indices.clear();
  sprite_sizes.clear();
}

void RenderingSystem::register_camera(Entity c) { camera = c; }

void RenderingSystem::update_transform(std::shared_ptr<Transform> transform,
                                       glm::vec3 o) {
  transform->model = glm::mat4(1.0f);
  transform->model =
      glm::translate(transform->model, transform->get_position_minus_offset());

  if (!transform->isometric) {
    if (transform->center) {
      transform->model = glm::translate(
          transform->model,
          glm::vec3(0.5 * transform->width, 0.5 * transform->height, 0.0));
    } else {
      if (transform->width != 0.0f && transform->height != 0.0f)
        transform->model = glm::translate(
            transform->model, glm::vec3(0.5 * transform->width, 0.0, 0.0));
    }
  }
  if (o != glm::vec3(0.0f))
    transform->model = glm::translate(transform->model, o);

  if (transform->flip)
    transform->model = glm::rotate(transform->model, glm::radians(180.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f));

  if (transform->isometric)
    transform->model =
        glm::rotate(transform->model, glm::radians(45.0f), glm::vec3(0, 0, 1));

  glm::mat4 rotation_matrix = transform->getRotationMatrix();
  transform->model = transform->model * rotation_matrix;

  if (transform->width != 0.0f && transform->height != 0.0f)
    transform->model = glm::scale(
        transform->model,
        glm::vec3(transform->width, transform->height, transform->width));

  glm::vec3 s = transform->get_scale();
  transform->model = glm::scale(transform->model, s);
}

void RenderingSystem::set_shader_transform_uniforms(
    std::shared_ptr<Shader> shader, std::shared_ptr<Transform> transform,
    std::shared_ptr<Camera> camera, int x, int y, float zoom, int tick) {
  if (shader != nullptr && transform != nullptr) {
    glUniformMatrix4fv(
        glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE,
        glm::value_ptr(camera->get_projection_matrix(x, y, zoom)));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                       glm::value_ptr(transform->model));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                       glm::value_ptr(camera->get_view_matrix()));
    glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
    glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
  }
}

void RenderingSystem::set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                                 std::shared_ptr<Sprite> sprite,
                                                 int tick,
                                                 glm::vec3 cam_position) {
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
  }
}

float RenderingSystem::get_text_width(Entity entity, ComponentManager &cm) {
  auto font = cm.get_component<Font>(entity);
  auto text = cm.get_component<Text>(entity);
  float width = 0.0f;
  for (auto c = text->text.begin(); c != text->text.end(); c++) {
    Glyph glyph = font->glyphs.find(*c)->second;
    width += glyph.advance >> 6;
  }
  return width;
}

float RenderingSystem::get_text_height(Entity entity, ComponentManager &cm) {
  auto font = cm.get_component<Font>(entity);
  auto text = cm.get_component<Text>(entity);
  float height = 0.0f;
  for (auto c = text->text.begin(); c != text->text.end(); c++) {
    Glyph glyph = font->glyphs.find(*c)->second;
    if (glyph.size.y > height)
      height = glyph.size.y;
  }
  return height;
}

float RenderingSystem::get_text_font_height(Entity entity,
                                            ComponentManager &cm) {
  auto font = cm.get_component<Font>(entity);
  auto text = cm.get_component<Text>(entity);
  if (!font || !text)
    return 0.0f;
  return font->line_height;
}

void RenderingSystem::sort(ComponentManager &cm) {
  std::stable_sort(registered_entities.begin(), registered_entities.end(),
                   [&](const Entity &a, const Entity &b) {
                     auto trans_a = cm.get_component<Transform>(a);
                     auto trans_b = cm.get_component<Transform>(b);
                     if (trans_a == nullptr || trans_b == nullptr)
                       return false;
                     return trans_a->position.z < trans_b->position.z;
                   });
}

void RenderingSystem::shadow_rotate(ComponentManager &cm) {
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

void RenderingSystem::shadow_reset_rotation(ComponentManager &cm) {
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

void RenderingSystem::pre_filter_lights(std::shared_ptr<Camera> cam) {
  filteredLightPositions.clear();
  filteredLightColors.clear();
  glm::vec3 camPos = cam->get_position();
  for (size_t i = 0; i < lightPositions.size(); ++i) {
    if (glm::distance(lightPositions[i], camPos) <= maxDistance) {
      filteredLightPositions.push_back(lightPositions[i]);
      filteredLightColors.push_back(lightColors[i]);
    }
  }
}

void RenderingSystem::calculate_lighting(std::shared_ptr<Shader> shader,
                                         std::shared_ptr<Camera> cam,
                                         int tick) {
  int dayTick = (tick >= 16) ? tick - 16 : 0;
  float angle = glm::mix(-glm::half_pi<float>(), glm::half_pi<float>(),
                         (float)dayTick / (TESettings::MAX_TICKS / 2.0));
  glUniform1f(glGetUniformLocation(shader->ID, "ambientStrength"),
              cos(angle) * 0.5f + 0.1f);
  glUniform3fv(glGetUniformLocation(shader->ID, "lightPos"), 1,
               glm::value_ptr(dirLightPos));
  glUniform3fv(glGetUniformLocation(shader->ID, "lightColor"), 1,
               glm::value_ptr(dirLightColor));
  glUniform3fv(glGetUniformLocation(shader->ID, "cameraPos"), 1,
               glm::value_ptr(cam->get_position()));

  if (!filteredLightPositions.empty()) {
    int numPointLights = (int)filteredLightPositions.size();
    glUniform1i(glGetUniformLocation(shader->ID, "numPointLights"),
                numPointLights);
    glUniform3fv(glGetUniformLocation(shader->ID, "pointLightPositions"),
                 numPointLights, glm::value_ptr(filteredLightPositions[0]));
    glUniform3fv(glGetUniformLocation(shader->ID, "pointLightColors"),
                 numPointLights, glm::value_ptr(filteredLightColors[0]));
  }
}

void RenderingSystem::render(ComponentManager &cm, int x, int y, float zoom,
                             int tick, float delta_time) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);
  if (cam != nullptr)
    pre_filter_lights(cam);

  for (auto &entity : registered_entities) {
    auto shader = cm.get_component<Shader>(entity);
    if (shader != nullptr) {
      shader->use();
    } else {
      continue;
    }
    auto transform = cm.get_component<Transform>(entity);
    if (transform != nullptr) {
      if (cam != nullptr)
        transform->update_camera(cam);
      update_transform(transform);
    }

    auto particle_emitter = cm.get_component<ParticleEmitter>(entity);
    if (particle_emitter != nullptr) {
      auto quad = cm.get_component<Quad>(entity);
      if (quad != nullptr) {
        set_shader_transform_uniforms(shader, transform, cam, x, y, zoom, tick);
        particle_emitter->update(delta_time);
        particle_emitter->render(quad, shader);
      }
      continue;
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
          calculate_lighting(root_shader, cam, tick);
          auto texture = cm.get_component<Texture>(entity);
          if (texture != nullptr)
            texture->render();
          model->render(rootModelBone->transform->model, delta_time);
        }
      }

      if (auto rootSpriteBone =
              std::dynamic_pointer_cast<SpriteBone>(animation->root)) {
        rootSpriteBone->sprite->index = rootSpriteBone->sprite_index;
        rootSpriteBone->sprite->update();
        rootSpriteBone->quad->render();
        if (rootSpriteBone->sprite->slice_shader != nullptr) {
          set_shader_sprite_uniforms(root_shader, rootSpriteBone->sprite, tick,
                                     cam->get_position());
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
          set_shader_transform_uniforms(bone_shader, bone->transform, cam, x, y,
                                        zoom, tick);
        }
        if (auto spriteBone = std::dynamic_pointer_cast<SpriteBone>(bone)) {
          if (spriteBone->sprite) {
            if (!spriteBone->sprite->hidden) {
              spriteBone->sprite->index = spriteBone->sprite_index;
              spriteBone->sprite->update();
              spriteBone->quad->render();
              if (spriteBone->sprite->slice_shader != nullptr) {
                spriteBone->sprite->render_sliced();
              } else {
                spriteBone->texture->render();
                set_shader_sprite_uniforms(bone_shader, spriteBone->sprite,
                                           tick, cam->get_position());
                spriteBone->sprite->render();
              }
            }
          }
        }
      }
    }

    auto text = cm.get_component<Text>(entity);
    if (text != nullptr) {
      if (text->center) {
        update_transform(transform,
                         glm::vec3(-get_text_width(entity, cm) * 0.5f,
                                   -get_text_font_height(entity, cm) * 0.5f,
                                   0.0f));
      }
    }
    if (text != nullptr) {
      auto font = cm.get_component<Font>(entity);
      if (font != nullptr) {
        if (!text->hidden) {
          if (!animation || (animation && !animation->playing))
            set_shader_transform_uniforms(shader, transform, cam, x, y, zoom,
                                          tick);
          text->render(font, 0.0f, 0.0f, get_text_font_height(entity, cm),
                       shader);
        }
      }
    }

    if (animation == nullptr || !animation->playing) {
      auto quad = cm.get_component<Quad>(entity);
      if (quad != nullptr)
        quad->render();

      auto sprite = cm.get_component<Sprite>(entity);
      if (sprite != nullptr) {
        if (!sprite->hidden) {
          sprite->update();
          if (sprite->slice_shader != nullptr) {
            sprite->render_sliced();
          } else {
            auto texture = cm.get_component<Texture>(entity);
            if (texture != nullptr) {
              set_shader_transform_uniforms(shader, transform, cam, x, y, zoom,
                                            tick);
              set_shader_sprite_uniforms(shader, sprite, tick,
                                         cam->get_position());
              texture->render();
              sprite->render();
            } else {
              std::cout << "No texture for sprite entity " << entity
                        << std::endl;
            }
          }
        }
      }

      auto model = cm.get_component<Model>(entity);
      if (model != nullptr) {
        BoundingSphere sphere = transform->get_bounding_sphere();
        if (is_in_frustum(cam->get_frustum(x, y, zoom), sphere)) {
          set_shader_transform_uniforms(shader, transform, cam, x, y, zoom,
                                        tick);
          calculate_lighting(shader, cam, tick);
          auto texture = cm.get_component<Texture>(entity);
          if (texture != nullptr)
            texture->render();

          bool use_skinning = false;
          GLint boneMatrixLoc =
              glGetUniformLocation(shader->ID, "boneMatrices");
          if (boneMatrixLoc >= 0) {
            auto skeletal_animation =
                cm.get_component<SkeletalAnimation>(entity);
            if (skeletal_animation != nullptr) {
              std::array<glm::mat4, 32> boneMatrices;
              size_t count = skeletal_animation->poseMatrices.size();
              for (size_t i = 0; i < 32; ++i)
                boneMatrices[i] = (i < count)
                                      ? skeletal_animation->poseMatrices[i]
                                      : glm::mat4(1.0f);
              glUniformMatrix4fv(boneMatrixLoc, 32, GL_FALSE,
                                 glm::value_ptr(boneMatrices[0]));
              use_skinning = true;
            }
          }
          model->render(transform->model, delta_time, use_skinning);
        }
      }
    }
  }
}

bool RenderingSystem::is_in_frustum(const Frustum &frustum,
                                    const BoundingSphere &sphere) {
  for (int i = 0; i < 6; ++i) {
    const glm::vec4 &plane = frustum.planes[i];
    float distance = glm::dot(glm::vec3(plane), sphere.center) + plane.w;
    if (distance < -(sphere.radius * 6.0f))
      return false;
  }
  return true;
}

void RenderingSystem::init_instanced_buffers(std::shared_ptr<Quad> quad,
                                             std::shared_ptr<Shader> shader,
                                             size_t max_instances) {
  quad->render();

  glGenBuffers(1, &instanceVBO);
  glGenBuffers(1, &instanceVBO2);
  glGenBuffers(1, &instanceVBO3);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::mat4), nullptr,
               GL_DYNAMIC_DRAW);

  GLint attrLoc1 = glGetAttribLocation(shader->ID, "aModel");
  for (int i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(attrLoc1 + i);
    glVertexAttribPointer(attrLoc1 + i, 4, GL_FLOAT, GL_FALSE,
                          sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
    glVertexAttribDivisor(attrLoc1 + i, 1);
  }

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
  glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(float), nullptr,
               GL_DYNAMIC_DRAW);
  GLint attrLoc2 = glGetAttribLocation(shader->ID, "aIndex");
  glEnableVertexAttribArray(attrLoc2);
  glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                        (void *)0);
  glVertexAttribDivisor(attrLoc2, 1);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
  glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::vec2), nullptr,
               GL_DYNAMIC_DRAW);
  GLint attrLoc3 = glGetAttribLocation(shader->ID, "aSpriteSize");
  glEnableVertexAttribArray(attrLoc3);
  glVertexAttribPointer(attrLoc3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
                        (void *)0);
  glVertexAttribDivisor(attrLoc3, 1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderingSystem::prepare_instanced(ComponentManager &cm, float x, float y,
                                        int zoom) {
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
    glBufferSubData(GL_ARRAY_BUFFER, 0, sprite_sizes.size() * sizeof(glm::vec2),
                    sprite_sizes.data());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderingSystem::instanced_render(ComponentManager &cm, int x, int y,
                                       std::shared_ptr<Quad> quad,
                                       std::shared_ptr<Texture> texture,
                                       std::shared_ptr<Shader> shader,
                                       float zoom, int tick, glm::vec4 color) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

  shader->use();
  quad->render();
  texture->render();

  glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
  glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1,
               glm::value_ptr(color));
  glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1,
               glm::value_ptr(glm::vec2(texture->width, texture->height)));

  glm::mat4 view = cam->get_view_matrix();
  glm::mat4 projection = cam->get_projection_matrix(x, y, zoom);
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
}

void RenderingSystem::init_instanced_model_buffers(
    std::shared_ptr<Model> model, std::shared_ptr<Shader> shader,
    size_t max_instances) {
  for (auto &mesh : model->meshes) {
    glBindVertexArray(mesh->VAO);

    if (!modelInstanceVBO)
      glGenBuffers(1, &modelInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::mat4), nullptr,
                 GL_DYNAMIC_DRAW);
    for (int i = 0; i < 4; ++i) {
      std::string name = "aModelMatrix" + std::to_string(i);
      GLint attrLoc = glGetAttribLocation(shader->ID, name.c_str());
      if (attrLoc >= 0) {
        glEnableVertexAttribArray(attrLoc);
        glVertexAttribPointer(attrLoc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void *)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(attrLoc, 1);
      }
    }

    if (!modelInstanceVBO2)
      glGenBuffers(1, &modelInstanceVBO2);
    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO2);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);
    GLint attrLoc2 = glGetAttribLocation(shader->ID, "aModelIndex");
    if (attrLoc2 >= 0) {
      glEnableVertexAttribArray(attrLoc2);
      glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                            (void *)0);
      glVertexAttribDivisor(attrLoc2, 1);
    }

    if (!modelInstanceVBO3)
      glGenBuffers(1, &modelInstanceVBO3);
    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO3);
    glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::vec4), nullptr,
                 GL_DYNAMIC_DRAW);
    GLint attrLoc3 = glGetAttribLocation(shader->ID, "aModelParams");
    if (attrLoc3 >= 0) {
      glEnableVertexAttribArray(attrLoc3);
      glVertexAttribPointer(attrLoc3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                            (void *)0);
      glVertexAttribDivisor(attrLoc3, 1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void RenderingSystem::prepare_instanced_models(ComponentManager &cm,
                                               std::shared_ptr<Shader> shader,
                                               float x, float y, int zoom) {
  modelMatrices.clear();
  modelIndices.clear();
  modelParams.clear();
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

  for (auto &entity : registered_entities) {
    auto transform = cm.get_component<Transform>(entity);
    if (transform != nullptr) {
      transform->update_camera(cam);
      update_transform(transform);
      modelMatrices.push_back(transform->model);
    }
    auto modelComp = cm.get_component<Model>(entity);
    if (modelComp != nullptr) {
      modelIndices.push_back(modelComp->index);
      modelParams.push_back(modelComp->params);
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO);
  if (!modelMatrices.empty())
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    modelMatrices.size() * sizeof(glm::mat4),
                    modelMatrices.data());

  glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO2);
  if (!modelIndices.empty())
    glBufferSubData(GL_ARRAY_BUFFER, 0, modelIndices.size() * sizeof(float),
                    modelIndices.data());

  glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO3);
  if (!modelParams.empty())
    glBufferSubData(GL_ARRAY_BUFFER, 0, modelParams.size() * sizeof(glm::vec4),
                    modelParams.data());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderingSystem::instanced_model_render(ComponentManager &cm, int x, int y,
                                             float zoom, int tick) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);
  if (cam != nullptr)
    pre_filter_lights(cam);

  std::unordered_map<std::shared_ptr<Model>, std::vector<Entity>> modelGroups;
  for (auto &entity : registered_entities) {
    auto modelComp = cm.get_component<Model>(entity);
    if (modelComp)
      modelGroups[modelComp].push_back(entity);
  }

  for (auto &[modelPtr, entities] : modelGroups) {
    modelMatrices.clear();
    modelIndices.clear();
    modelParams.clear();

    for (auto &entity : entities) {
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        transform->update_camera(cam);
        update_transform(transform);
        modelMatrices.push_back(transform->model);
      }
      auto modelComp = cm.get_component<Model>(entity);
      if (modelComp) {
        modelIndices.push_back(modelComp->index);
        modelParams.push_back(modelComp->params);
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO);
    if (!modelMatrices.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0,
                      modelMatrices.size() * sizeof(glm::mat4),
                      modelMatrices.data());

    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO2);
    if (!modelIndices.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0, modelIndices.size() * sizeof(float),
                      modelIndices.data());

    glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO3);
    if (!modelParams.empty())
      glBufferSubData(GL_ARRAY_BUFFER, 0,
                      modelParams.size() * sizeof(glm::vec4),
                      modelParams.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto shader = cm.get_component<Shader>(entities[0]);
    if (shader) {
      shader->use();

      glm::mat4 view = cam->get_view_matrix();
      glm::mat4 projection = cam->get_projection_matrix(x, y, zoom);
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                         GL_FALSE, glm::value_ptr(projection));
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                         glm::value_ptr(view));
      glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
      glUniform1f(glGetUniformLocation(shader->ID, "jitter"), inst_jitter);
      glUniform1f(glGetUniformLocation(shader->ID, "jitter_speed"),
                  inst_jitter_speed);

      for (auto &mesh : modelPtr->meshes) {
        glBindVertexArray(mesh->VAO);

        // Re-bind our own instance VBOs
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO);
        for (int i = 0; i < 4; ++i) {
          std::string name = "aModelMatrix" + std::to_string(i);
          GLint loc = glGetAttribLocation(shader->ID, name.c_str());
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                  (void *)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO2);
        {
          GLint loc = glGetAttribLocation(shader->ID, "aModelIndex");
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                                  (void *)0);
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO3);
        {
          GLint loc = glGetAttribLocation(shader->ID, "aModelParams");
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                                  (void *)0);
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        calculate_lighting(shader, cam, tick);
        glUniform3fv(glGetUniformLocation(shader->ID, "materialDiffuse"), 1,
                     glm::value_ptr(mesh->diffuseColor));
        glUniform3fv(glGetUniformLocation(shader->ID, "albedo"), 1,
                     glm::value_ptr(mesh->diffuseColor));
        glUniform1f(glGetUniformLocation(shader->ID, "metallic"), 0.1f);
        glUniform1f(glGetUniformLocation(shader->ID, "roughness"), 0.1f);
        glUniform3fv(glGetUniformLocation(shader->ID, "materialSpecular"), 1,
                     glm::value_ptr(mesh->specularColor));
        glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
        glDrawElementsInstanced(GL_TRIANGLES,
                                static_cast<unsigned int>(mesh->indices.size()),
                                GL_UNSIGNED_INT, 0, entities.size());
        glBindVertexArray(0);
      }
    }
  }
}
