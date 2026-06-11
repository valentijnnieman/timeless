#include "timeless/systems/rendering_system.hpp"
#include <cmath>

// ---------------------------------------------------------------------------
// Shadow map setup
// ---------------------------------------------------------------------------

void RenderingSystem::setup_shadow_map() {
  glGenFramebuffers(1, &shadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

  glGenTextures(1, &shadowDepthTex);
  glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
               shadowMapSize, shadowMapSize, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         GL_TEXTURE_2D, shadowDepthTex, 0);
  // Depth-only FBO: explicitly declare no colour output
  const GLenum none = GL_NONE;
  glDrawBuffers(1, &none);
  glReadBuffer(GL_NONE);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
    std::cerr << "Shadow FBO incomplete: 0x" << std::hex << status << std::endl;

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ---------------------------------------------------------------------------
// Light-space matrix for the directional light
// ---------------------------------------------------------------------------

glm::mat4 RenderingSystem::compute_light_space_matrix() const {
  // dirLightPos is the direction the light *points toward*; the source sits
  // in the opposite direction, far from the scene centre.
  const glm::vec3 sceneCenter = shadowCenter;
  glm::vec3 lightDir = glm::normalize(dirLightPos);
  glm::vec3 lightEye = sceneCenter - lightDir * 3000.0f;

  // Choose an up vector that is not collinear with lightDir
  glm::vec3 up(0.0f, -1.0f, 0.0f);
  if (std::abs(glm::dot(lightDir, up)) > 0.99f)
    up = glm::vec3(1.0f, 0.0f, 0.0f);

  glm::mat4 lightView = glm::lookAt(lightEye, sceneCenter, up);

  // Orthographic volume that covers the whole scene
  const float orthoSize = shadowOrthoSize;
  glm::mat4 lightProj = glm::ortho(
      -orthoSize,  orthoSize,   // left, right
      -orthoSize,  orthoSize,   // bottom, top
      1.0f,        7000.0f      // near, far
  );

  return lightProj * lightView;
}

// ---------------------------------------------------------------------------
// Shadow pre-pass: render scene from light's perspective into shadowFBO
// ---------------------------------------------------------------------------

void RenderingSystem::render_shadow_pass(ComponentManager &cm,
                                         std::shared_ptr<Camera> cam,
                                         float delta_time) {
  // Save GL state
  GLint savedFBO = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &savedFBO);
  GLint savedVP[4];
  glGetIntegerv(GL_VIEWPORT, savedVP);

  lightSpaceMatrix = compute_light_space_matrix();

  glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
  glViewport(0, 0, shadowMapSize, shadowMapSize);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  shadowDepthShader->use();
  glUniformMatrix4fv(
      shadowDepthShader->get_uniform("lightSpaceMatrix"),
      1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

  for (auto &entity : registered_entities) {
    if (no_shadow_cast.count(entity)) continue; // excluded from casting shadows

    std::shared_ptr<Model>     mdl;
    std::shared_ptr<Transform> xfm;

    // Animated (position/rotation) model bone
    auto anim = cm.get_component<Animation>(entity);
    if (anim && anim->playing) {
      if (auto rootBone = std::dynamic_pointer_cast<ModelBone>(anim->root)) {
        mdl = rootBone->model;
        xfm = rootBone->transform;
        if (xfm) {
          if (cam) xfm->update_camera(cam);
          update_transform(xfm);
        }
      }
    }

    // Static / non-animated model
    if (!mdl) {
      mdl = cm.get_component<Model>(entity);
      xfm = cm.get_component<Transform>(entity);
      if (xfm) {
        if (cam) xfm->update_camera(cam);
        update_transform(xfm);
      }
    }

    if (!mdl || !xfm || mdl->hidden) continue;

    // Upload bone matrices once per entity to the depth shader.
    bool hasSkinning = upload_bone_matrices(entity, cm, shadowDepthShader);

    // Iterate meshes directly so all uniform lookups use shadowDepthShader->ID.
    // Model::render() uses this->shader->ID (the cooktor shader) which would
    // set uniforms on the wrong program when the depth shader is active.
    for (auto &mesh : mdl->meshes) {
      glBindVertexArray(mesh->VAO);

      // Mirror Model::render()'s per-mesh matrix logic
      glm::mat4 meshModelMatrix;
      if (hasSkinning && !mesh->boneInfos.empty()) {
        meshModelMatrix = xfm->model;
      } else {
        glm::mat4 animOffset = glm::translate(glm::mat4(1.0f), mesh->position);
        meshModelMatrix = xfm->model * animOffset;
      }

      glUniformMatrix4fv(
          shadowDepthShader->get_uniform("model"),
          1, GL_FALSE, glm::value_ptr(meshModelMatrix));
      glUniform1i(
          shadowDepthShader->get_uniform("useSkinning"),
          (hasSkinning && !mesh->boneInfos.empty()) ? 1 : 0);

      glDrawElements(GL_TRIANGLES,
                     static_cast<unsigned int>(mesh->indices.size()),
                     GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
  }

  // Restore GL state
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(savedFBO));
  glViewport(savedVP[0], savedVP[1], savedVP[2], savedVP[3]);
}

bool RenderingSystem::upload_bone_matrices(Entity entity, ComponentManager &cm,
                                           std::shared_ptr<Shader> shader) {
  GLint boneMatrixLoc = shader->get_uniform("boneMatrices");
  if (boneMatrixLoc < 0) return false;
  auto skeletal_animation = cm.get_component<SkeletalAnimation>(entity);
  if (skeletal_animation == nullptr) return false;
  std::array<glm::mat4, 128> boneMatrices;
  size_t count = skeletal_animation->poseMatrices.size();
  for (size_t i = 0; i < 128; ++i)
    boneMatrices[i] = (i < count) ? skeletal_animation->poseMatrices[i]
                                  : glm::mat4(1.0f);
  glUniformMatrix4fv(boneMatrixLoc, 128, GL_FALSE,
                     glm::value_ptr(boneMatrices[0]));
  return true;
}

void RenderingSystem::purge(ComponentManager &cm) {
  // for (auto &ent : registered_entities)
  //   cm.remove_entity(ent);
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
    const glm::mat4& view, const glm::mat4& projection, float time, int tick) {
  if (shader != nullptr && transform != nullptr) {
    glUniformMatrix4fv(shader->get_uniform("projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniformMatrix4fv(shader->get_uniform("model"), 1, GL_FALSE,
                       glm::value_ptr(transform->model));
    glUniformMatrix4fv(shader->get_uniform("view"), 1, GL_FALSE,
                       glm::value_ptr(view));
    glUniform1f(shader->get_uniform("time"), time);
    glUniform1f(shader->get_uniform("tick"), tick);
  }
}

void RenderingSystem::set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                                 std::shared_ptr<Sprite> sprite,
                                                 float time,
                                                 int tick,
                                                 glm::vec3 cam_position) {
  if (shader != nullptr && sprite != nullptr) {
    float cols = sprite->spriteSheetSize.x / sprite->spriteSize.x;
    float col = sprite->index - cols * floor(sprite->index / cols);
    float row = floor(sprite->index / cols);

    glUniform1i(shader->get_uniform("texture1"), 0);
    glUniform1f(shader->get_uniform("index"),
                static_cast<float>(sprite->index));
    glUniform1f(shader->get_uniform("col"), col);
    glUniform1f(shader->get_uniform("row"), row);
    glUniform4fv(shader->get_uniform("highlightColor"), 1,
                 glm::value_ptr(sprite->color));
    glUniform2fv(shader->get_uniform("spriteSheetSize"), 1,
                 glm::value_ptr(sprite->spriteSheetSize));
    glUniform2fv(shader->get_uniform("spriteSize"), 1,
                 glm::value_ptr(sprite->spriteSize));
    glUniform1f(shader->get_uniform("time"), time);
    glUniform1f(shader->get_uniform("jitter"), ui_jitter);
    glUniform1f(shader->get_uniform("jitter_speed"),
                ui_jitter_speed);
    glUniform1f(shader->get_uniform("tick"), tick);
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
                                         const glm::vec3& cam_pos,
                                         int tick) {
  int dayTick = (tick >= 16) ? tick - 16 : 0;
  float angle = glm::mix(-glm::half_pi<float>(), glm::half_pi<float>(),
                         (float)dayTick / (TESettings::MAX_TICKS / 2.0));
  float ambient = (ambientStrength >= 0.0f) ? ambientStrength : cos(angle) * 0.5f + 0.1f;
  glUniform1f(shader->get_uniform("ambientStrength"), ambient);
  glUniform3fv(shader->get_uniform("lightPos"), 1,
               glm::value_ptr(dirLightPos));
  glUniform3fv(shader->get_uniform("lightColor"), 1,
               glm::value_ptr(dirLightColor));
  glUniform3fv(shader->get_uniform("cameraPos"), 1,
               glm::value_ptr(cam_pos));

  int numPointLights = (int)filteredLightPositions.size();
  glUniform1i(shader->get_uniform("numPointLights"), numPointLights);
  if (numPointLights > 0) {
    glUniform3fv(shader->get_uniform("pointLightPositions"),
                 numPointLights, glm::value_ptr(filteredLightPositions[0]));
    glUniform3fv(shader->get_uniform("pointLightColors"),
                 numPointLights, glm::value_ptr(filteredLightColors[0]));
  }

  // Shadow map uniforms
  glUniformMatrix4fv(
      shader->get_uniform("lightSpaceMatrix"),
      1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
  if (shadowDepthTex != 0) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glUniform1i(shader->get_uniform("shadowMap"), 1);
    glActiveTexture(GL_TEXTURE0);
  }
}

void RenderingSystem::render(ComponentManager &cm, int x, int y, float zoom,
                             int tick, float delta_time) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);
  if (cam != nullptr)
    pre_filter_lights(cam);

  // Cache per-frame values once — avoids redundant matrix computations and
  // system calls inside the per-entity loop.
  const float time = (float)glfwGetTime();
  glm::mat4 view{}, projection{};
  glm::vec3 cam_pos{};
  Frustum frustum{};
  if (cam != nullptr) {
    view       = cam->get_view_matrix();
    projection = cam->get_projection_matrix(x, y, zoom);
    cam_pos    = cam->get_position();
    frustum    = cam->get_frustum(x, y, zoom);
  }

  // Lazy shadow-map setup + shadow pre-pass
  if (shadowDepthShader) {
    if (shadowFBO == 0)
      setup_shadow_map();
    render_shadow_pass(cm, cam, delta_time);
  }

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
        set_shader_transform_uniforms(shader, transform, view, projection, time, tick);
        // Don't let the (soft, fading) particles write depth so they don't
        // occlude each other or the scene. Blending is already enabled globally
        // (window_manager) with the same func, so leave it on — disabling it
        // here would break the text pass that renders afterwards.
        glDepthMask(GL_FALSE);
        particle_emitter->update(delta_time);
        particle_emitter->render(quad, shader);
        glDepthMask(GL_TRUE);
      }
      continue;
    }

    auto animation = cm.get_component<Animation>(entity);
    if (animation != nullptr && animation->playing) {
      animation->root->transform->update_camera(cam);
      update_transform(animation->root->transform);
      auto root_shader = cm.get_component<Shader>(animation->root->entity);
      set_shader_transform_uniforms(root_shader, animation->root->transform,
                                    view, projection, time, tick);

      if (auto rootModelBone =
              std::dynamic_pointer_cast<ModelBone>(animation->root)) {
        auto model = rootModelBone->model;
        if (model != nullptr) {
          calculate_lighting(root_shader, cam_pos, tick);
          auto texture = cm.get_component<Texture>(entity);
          if (texture != nullptr)
            texture->render();
          model->render(rootModelBone->transform->model, delta_time,
                        upload_bone_matrices(entity, cm, root_shader));
        }
      }

      if (auto rootSpriteBone =
              std::dynamic_pointer_cast<SpriteBone>(animation->root)) {
        if (rootSpriteBone->sprite->hidden) continue;
        rootSpriteBone->sprite->index = rootSpriteBone->sprite_index;
        rootSpriteBone->sprite->update();
        rootSpriteBone->quad->render();
        if (rootSpriteBone->sprite->slice_shader != nullptr) {
          set_shader_sprite_uniforms(root_shader, rootSpriteBone->sprite, time,
                                     tick, cam_pos);
          rootSpriteBone->sprite->render_sliced();
        } else {
          if (rootSpriteBone->texture != nullptr) {
            rootSpriteBone->texture->render();
            set_shader_sprite_uniforms(root_shader, rootSpriteBone->sprite,
                                       time, tick, cam_pos);
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
          set_shader_transform_uniforms(bone_shader, bone->transform, view,
                                        projection, time, tick);
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
                                           time, tick, cam_pos);
                spriteBone->sprite->render();
              }
            }
          }
        }
      }
    }

    auto text = cm.get_component<Text>(entity);
    if (text != nullptr) {
      auto font = cm.get_component<Font>(entity);
      if (font != nullptr && !text->hidden) {
        // Centering shifts the glyphs left/up by half the text's extents so the
        // transform position lands at the text's middle.
        glm::vec3 offset(0.0f);
        if (text->center)
          offset = glm::vec3(-get_text_width(entity, cm) * 0.5f,
                             -get_text_font_height(entity, cm) * 0.5f, 0.0f);

        // While an animation drives this text (e.g. the zoom-in countdown), the
        // transform that actually gets rendered is the animation's root copy,
        // and its model was already uploaded above WITHOUT this centering
        // offset. Previously the offset was folded into the entity's own
        // transform and then never re-uploaded (the gate below skipped it during
        // animation), so centered + animated text rendered off-center. Build the
        // model from whichever transform is animating, fold in the offset, and
        // upload it right before drawing the glyphs.
        auto draw_transform = (animation && animation->playing)
                                  ? animation->root->transform
                                  : transform;
        shader->use();
        update_transform(draw_transform, offset);
        set_shader_transform_uniforms(shader, draw_transform, view, projection,
                                      time, tick);
        text->render(font, 0.0f, 0.0f, get_text_font_height(entity, cm),
                     shader);
      }
    }

    if (animation == nullptr || !animation->playing) {
      auto quad = cm.get_component<Quad>(entity);
      if (quad != nullptr) {
        quad->render(); // binds the VAO (unchanged behaviour)
        // A standalone quad (no Sprite/Model on this entity, e.g. a ground
        // decal): position it from its Transform and actually draw it. This path
        // previously only bound the VAO — it set no transform uniforms and
        // issued no draw call, so standalone quads never rendered.
        if (cm.get_component<Sprite>(entity) == nullptr &&
            cm.get_component<Model>(entity) == nullptr) {
          set_shader_transform_uniforms(shader, transform, view, projection, time,
                                        tick);
          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      }

      auto sprite = cm.get_component<Sprite>(entity);
      if (sprite != nullptr) {
        if (!sprite->hidden) {
          sprite->update();
          if (sprite->slice_shader != nullptr) {
            sprite->render_sliced();
          } else {
            auto texture = cm.get_component<Texture>(entity);
            if (texture != nullptr) {
              set_shader_transform_uniforms(shader, transform, view, projection,
                                            time, tick);
              set_shader_sprite_uniforms(shader, sprite, time, tick, cam_pos);
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
        if (is_in_frustum(frustum, sphere)) {
          set_shader_transform_uniforms(shader, transform, view, projection,
                                        time, tick);
          calculate_lighting(shader, cam_pos, tick);
          auto texture = cm.get_component<Texture>(entity);
          if (texture != nullptr)
            texture->render();

          // Per-entity sun-burn uniforms — the fragment shader reddens only the
          // sunlit fragments. Set 0 for entities without a burn so the previous
          // entity's value doesn't leak (get_uniform is a no-op == -1 for
          // shaders without these uniforms).
          auto burnIt = entity_burn.find(entity);
          float burnAmt = (burnIt != entity_burn.end()) ? burnIt->second.w : 0.0f;
          glm::vec3 burnCol = (burnIt != entity_burn.end())
                                  ? glm::vec3(burnIt->second)
                                  : glm::vec3(1.0f);
          glUniform1f(shader->get_uniform("burnAmount"), burnAmt);
          glUniform3fv(shader->get_uniform("burnColor"), 1,
                       glm::value_ptr(burnCol));
          auto tanIt = entity_tan.find(entity);
          float tanAmt = (tanIt != entity_tan.end()) ? tanIt->second.w : 0.0f;
          glm::vec3 tanCol = (tanIt != entity_tan.end())
                                 ? glm::vec3(tanIt->second)
                                 : glm::vec3(1.0f);
          glUniform1f(shader->get_uniform("tanAmount"), tanAmt);
          glUniform3fv(shader->get_uniform("tanColor"), 1,
                       glm::value_ptr(tanCol));
          auto sheenIt = entity_sheen.find(entity);
          glUniform1f(shader->get_uniform("sheen"),
                      sheenIt != entity_sheen.end() ? sheenIt->second : 0.0f);
          // Per-entity pants tint (white = unchanged); the model's per-mesh
          // isPants uniform decides which mesh it actually affects.
          auto pantsIt = entity_pants.find(entity);
          glm::vec3 pantsCol = (pantsIt != entity_pants.end()) ? pantsIt->second
                                                               : glm::vec3(1.0f);
          glUniform3fv(shader->get_uniform("pantsColor"), 1,
                       glm::value_ptr(pantsCol));
          // Per-entity translucency: output as fragment alpha, and for a
          // translucent entity drop depth writes so it doesn't occlude geometry
          // behind it (depth test stays on, so it's still hidden by nearer
          // opaque geometry). Restore the depth mask afterwards.
          auto alphaIt = entity_alpha.find(entity);
          float modelAlpha =
              (alphaIt != entity_alpha.end()) ? alphaIt->second : 1.0f;
          glUniform1f(shader->get_uniform("modelAlpha"), modelAlpha);
          bool translucent = modelAlpha < 0.999f;
          if (translucent)
            glDepthMask(GL_FALSE);
          model->render(transform->model, delta_time,
                        upload_bone_matrices(entity, cm, shader));
          if (translucent)
            glDepthMask(GL_TRUE);
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

  GLint attrLoc1 = shader->get_attrib("aModel");
  for (int i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(attrLoc1 + i);
    glVertexAttribPointer(attrLoc1 + i, 4, GL_FLOAT, GL_FALSE,
                          sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
    glVertexAttribDivisor(attrLoc1 + i, 1);
  }

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
  glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(float), nullptr,
               GL_DYNAMIC_DRAW);
  GLint attrLoc2 = shader->get_attrib("aIndex");
  glEnableVertexAttribArray(attrLoc2);
  glVertexAttribPointer(attrLoc2, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                        (void *)0);
  glVertexAttribDivisor(attrLoc2, 1);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
  glBufferData(GL_ARRAY_BUFFER, max_instances * sizeof(glm::vec2), nullptr,
               GL_DYNAMIC_DRAW);
  GLint attrLoc3 = shader->get_attrib("aSpriteSize");
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

  glUniform1i(shader->get_uniform("texture1"), 0);
  glUniform4fv(shader->get_uniform("highlightColor"), 1,
               glm::value_ptr(color));
  glUniform2fv(shader->get_uniform("spriteSheetSize"), 1,
               glm::value_ptr(glm::vec2(texture->width, texture->height)));

  const float time = (float)glfwGetTime();
  glm::mat4 view = cam->get_view_matrix();
  glm::mat4 projection = cam->get_projection_matrix(x, y, zoom);
  glUniformMatrix4fv(shader->get_uniform("projection"), 1,
                     GL_FALSE, glm::value_ptr(projection));
  glUniformMatrix4fv(shader->get_uniform("view"), 1, GL_FALSE,
                     glm::value_ptr(view));
  glUniform1f(shader->get_uniform("time"), time);
  glUniform1f(shader->get_uniform("tick"), tick);
  glUniform1f(shader->get_uniform("jitter"), inst_jitter);
  glUniform1f(shader->get_uniform("jitter_speed"),
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
      GLint attrLoc = shader->get_attrib(name);
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
    GLint attrLoc2 = shader->get_attrib("aModelIndex");
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
    GLint attrLoc3 = shader->get_attrib("aModelParams");
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

  const float time = (float)glfwGetTime();
  glm::mat4 view{}, projection{};
  glm::vec3 cam_pos{};
  if (cam != nullptr) {
    view       = cam->get_view_matrix();
    projection = cam->get_projection_matrix(x, y, zoom);
    cam_pos    = cam->get_position();
  }

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

      glUniformMatrix4fv(shader->get_uniform("projection"), 1,
                         GL_FALSE, glm::value_ptr(projection));
      glUniformMatrix4fv(shader->get_uniform("view"), 1, GL_FALSE,
                         glm::value_ptr(view));
      glUniform1f(shader->get_uniform("time"), time);
      glUniform1f(shader->get_uniform("tick"), tick);
      glUniform1f(shader->get_uniform("jitter"), inst_jitter);
      glUniform1f(shader->get_uniform("jitter_speed"),
                  inst_jitter_speed);

      // Lighting is the same for all meshes in this group — set it once.
      calculate_lighting(shader, cam_pos, tick);

      for (auto &mesh : modelPtr->meshes) {
        glBindVertexArray(mesh->VAO);

        // Re-bind our own instance VBOs
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO);
        for (int i = 0; i < 4; ++i) {
          std::string name = "aModelMatrix" + std::to_string(i);
          GLint loc = shader->get_attrib(name);
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                  (void *)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO2);
        {
          GLint loc = shader->get_attrib("aModelIndex");
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                                  (void *)0);
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, modelInstanceVBO3);
        {
          GLint loc = shader->get_attrib("aModelParams");
          if (loc >= 0) {
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                                  (void *)0);
            glVertexAttribDivisor(loc, 1);
          }
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUniform3fv(shader->get_uniform("materialDiffuse"), 1,
                     glm::value_ptr(mesh->diffuseColor));
        glUniform3fv(shader->get_uniform("albedo"), 1,
                     glm::value_ptr(mesh->diffuseColor));
        glUniform1f(shader->get_uniform("metallic"),
                    modelPtr->metallic.value_or(mesh->metallic.value_or(0.1f)));
        glUniform1f(shader->get_uniform("roughness"),
                    modelPtr->roughness.value_or(mesh->roughness.value_or(0.1f)));
        glUniform3fv(shader->get_uniform("materialSpecular"), 1,
                     glm::value_ptr(mesh->specularColor));
        glUniform1i(shader->get_uniform("texture1"), 0);
        glDrawElementsInstanced(GL_TRIANGLES,
                                static_cast<unsigned int>(mesh->indices.size()),
                                GL_UNSIGNED_INT, 0, entities.size());
        glBindVertexArray(0);
      }
    }
  }
}
