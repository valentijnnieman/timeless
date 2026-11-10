#include "timeless/systems/rendering_system.hpp"
#include "timeless/components/shader_uniforms.hpp"
#include "timeless/input.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

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
  glBindFramebuffer(GL_FRAMEBUFFER, TE::screen_framebuffer);
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
  skeletal_animation->ensure_pose();
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

  // Turn upright-on-screen quads with the camera, about their anchor, before
  // any centering so centered text and sprites turn as a whole.
  if (transform->face_camera_yaw && camera_yaw != 0.0f)
    transform->model =
        glm::rotate(transform->model, camera_yaw, glm::vec3(0, 0, 1));

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

void RenderingSystem::pre_filter_lights(std::shared_ptr<Camera> cam, int x, int y,
                                        float zoom) {
  filteredLightPositions.clear();
  filteredLightColors.clear();
  // Measured from the spot on the ground (z = 0) in the middle of the screen,
  // not the camera itself: an orthographic camera can sit anywhere along its
  // view line, and from far enough away no light was in range at all. Found
  // like mouse picking does, from the camera's own matrices (with a focus
  // point it doesn't look along get_forward()). The range reaches the
  // screen's corners (plus a light's reach) when zoomed out.
  glm::vec3 centre = cam->get_position();
  {
    glm::vec4 viewport(0.0f, 0.0f, float(x), float(y));
    glm::mat4 view = cam->get_view_matrix();
    glm::mat4 proj = cam->get_projection_matrix(x, y, zoom);
    glm::vec3 mid(0.5f * x, 0.5f * y, 0.0f);
    glm::vec3 near_point = glm::unProject(mid, view, proj, viewport);
    mid.z = 1.0f;
    glm::vec3 dir = glm::unProject(mid, view, proj, viewport) - near_point;
    if (std::abs(dir.z) > 1e-6f) {
      float t = -near_point.z / dir.z;
      centre = near_point + dir * t;
    }
  }
  float range = cam->perspective
                    ? maxDistance
                    : std::max(maxDistance, 0.5f * std::hypot(float(x), float(y)) * zoom + 600.0f);
  std::vector<std::pair<float, size_t>> in_range;
  for (size_t i = 0; i < lightPositions.size(); ++i) {
    glm::vec2 off = glm::vec2(lightPositions[i]) - glm::vec2(centre);
    float d = glm::length(off);
    if (d <= range)
      in_range.push_back({d, i});
  }
  // The shaders only hold maxPointLights; beyond that, extra lights would be
  // dropped arbitrarily (and the shader loop would read past its arrays), so
  // keep the nearest.
  if (in_range.size() > maxPointLights) {
    std::partial_sort(in_range.begin(), in_range.begin() + maxPointLights,
                      in_range.end());
    in_range.resize(maxPointLights);
  }
  for (auto &[d, i] : in_range) {
    filteredLightPositions.push_back(lightPositions[i]);
    filteredLightColors.push_back(lightColors[i]);
  }
}

float RenderingSystem::daylight(int tick) const {
  // Smooth 0 -> 1 across a twilight window centred on `edge`.
  auto rise = [&](int edge) {
    float half = std::max(twilightTicks, 1) * 0.5f;
    return glm::smoothstep(edge - half, edge + half, (float)tick);
  };
  float up = sunriseTick >= 0 ? rise(sunriseTick) : 1.0f;
  float down = sunsetTick >= 0 ? 1.0f - rise(sunsetTick) : 1.0f;
  return up * down;
}

void RenderingSystem::calculate_lighting(std::shared_ptr<Shader> shader,
                                         const glm::vec3& cam_pos,
                                         int tick) {
  int dayTick = (tick >= 16) ? tick - 16 : 0;
  float angle = glm::mix(-glm::half_pi<float>(), glm::half_pi<float>(),
                         (float)dayTick / (TESettings::MAX_TICKS / 2.0));
  float day = daylight(tick);
  float ambient = ambientStrength;
  if (ambient < 0.0f) {
    // The day curve dips below zero late in the day (cos past pi/2), so keep
    // it at least the night level, and blend to that level as the sun sets.
    float day_ambient = std::max((float)cos(angle) * 0.5f + 0.1f, nightAmbient);
    ambient = glm::mix(nightAmbient, day_ambient, day);
  }
  glm::vec3 sun = dirLightColor * day;
  glUniform1f(shader->get_uniform("ambientStrength"), ambient);
  glUniform3fv(shader->get_uniform("lightPos"), 1,
               glm::value_ptr(dirLightPos));
  glUniform3fv(shader->get_uniform("lightColor"), 1,
               glm::value_ptr(sun));
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
    pre_filter_lights(cam, x, y, zoom);

  // Cache per-frame values once — avoids redundant matrix computations and
  // system calls inside the per-entity loop.
  const float time = (float)TE::now_seconds();
  glm::mat4 view{}, projection{};
  glm::vec3 cam_pos{};
  Frustum frustum{};
  if (cam != nullptr) {
    view       = cam->get_view_matrix();
    camera_yaw = yaw_of(cam);
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
    auto transform = cm.get_component<Transform>(entity);
    // Off screen: skip it before anything else (with a crowd of citizens,
    // their bodies, icons and labels are most of what's registered, and most
    // are off screen). Queued tweens and particles still advance, as they
    // would have if it had been drawn.
    if (cam != nullptr && transform != nullptr &&
        !on_screen(cm, entity, transform, frustum)) {
      if (transform->has_queued_frames())
        update_transform(transform);
      if (auto emitter = cm.get_component<ParticleEmitter>(entity))
        emitter->update(delta_time);
      continue;
    }
    auto shader = cm.get_component<Shader>(entity);
    if (shader != nullptr) {
      shader->use();
    } else {
      continue;
    }
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

          // Per-entity shader uniform overrides (game-specific tints,
          // translucency, ...) live on an optional ShaderUniforms component, so
          // the engine stays ignorant of what they mean — it just uploads the
          // name/value pairs. For a translucent entity drop depth writes so it
          // doesn't occlude geometry behind it (depth test stays on, so it's
          // still hidden by nearer opaque geometry). Restore the mask afterwards.
          bool translucent = false;
          if (auto su = cm.get_component<ShaderUniforms>(entity)) {
            for (auto &[name, v] : su->floats)
              glUniform1f(shader->get_uniform(name), v);
            for (auto &[name, v] : su->vec3s)
              glUniform3fv(shader->get_uniform(name), 1, glm::value_ptr(v));
            translucent = su->translucent;
          }
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

bool RenderingSystem::on_screen(ComponentManager &cm, Entity entity,
                                const std::shared_ptr<Transform> &transform,
                                const Frustum &frustum) {
  // How far outside the frustum t's position is (0 if inside), and its
  // radius: a sprite is drawn 2 width by 2 height, a model is its scale times
  // its own few units (is_in_frustum allows 6).
  auto outside = [&](const std::shared_ptr<Transform> &t, float &radius) {
    glm::vec3 p = t->peek_position(); // not get_position(): that pops a frame
    p.x -= t->offset.x;
    p.y -= t->offset.y;
    float size = std::max({std::abs(t->width), std::abs(t->height),
                           glm::compMax(glm::abs(t->scale))});
    radius = size * 6.0f + 64.0f;
    float out = 0.0f;
    for (int i = 0; i < 6; ++i) {
      const glm::vec4 &plane = frustum.planes[i];
      out = std::max(out, -(glm::dot(glm::vec3(plane), p) + plane.w));
    }
    return out;
  };
  float radius = 0.0f;
  float out = outside(transform, radius);
  if (out <= radius)
    return true;
  // Text runs on from its anchor, and while an animation plays it's drawn
  // at the animation's root; only looked up near the edge, so the many
  // entities far off screen cost no more than the above.
  constexpr float near_edge = 2000.0f;
  if (out > radius + near_edge)
    return false;
  if (auto text = cm.get_component<Text>(entity))
    if (out <= radius + 40.0f * float(text->text.size()) * std::max(1.0f, std::abs(transform->width)))
      return true;
  auto animation = cm.get_component<Animation>(entity);
  if (animation != nullptr && animation->playing && animation->root != nullptr &&
      animation->root->transform != nullptr) {
    float root_radius = 0.0f;
    if (outside(animation->root->transform, root_radius) <= root_radius)
      return true;
  }
  return false;
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
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4),
                 models.data(), GL_STREAM_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO2);
  if (!sprite_indices.empty())
    glBufferData(GL_ARRAY_BUFFER, sprite_indices.size() * sizeof(float),
                 sprite_indices.data(), GL_STREAM_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO3);
  if (!sprite_sizes.empty())
    glBufferData(GL_ARRAY_BUFFER, sprite_sizes.size() * sizeof(glm::vec2),
                 sprite_sizes.data(), GL_STREAM_DRAW);

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

  const float time = (float)TE::now_seconds();
  glm::mat4 view = cam->get_view_matrix();
  camera_yaw = yaw_of(cam);
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

// The instance buffers are per model group now (see InstanceGroup), created and
// sized by prepare_instanced_models, and instanced_model_render points each
// mesh's VAO at them before drawing. Nothing to set up ahead of time; kept so
// callers that still call it compile.
void RenderingSystem::init_instanced_model_buffers(
    std::shared_ptr<Model> model, std::shared_ptr<Shader> shader,
    size_t max_instances) {}

// Upload `bytes` of `data` to the start of `vbo`, first reallocating it to
// `capacity_bytes` when that's not 0 (the buffer has to grow).
static void upload_instance_buffer(unsigned int vbo, const void *data,
                                   size_t bytes, size_t capacity_bytes) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  if (capacity_bytes > 0)
    glBufferData(GL_ARRAY_BUFFER, capacity_bytes, nullptr, GL_DYNAMIC_DRAW);
  if (bytes > 0)
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, data);
}

void RenderingSystem::prepare_instanced_models(ComponentManager &cm,
                                               std::shared_ptr<Shader> shader,
                                               float x, float y, int zoom) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);

  for (auto &[key, group] : instance_groups)
    group.entities.clear();
  for (auto &entity : registered_entities) {
    auto modelComp = cm.get_component<Model>(entity);
    if (!modelComp)
      continue;
    auto &group = instance_groups[modelComp.get()];
    group.model = modelComp;
    group.entities.push_back(entity);
  }

  for (auto it = instance_groups.begin(); it != instance_groups.end();) {
    auto &group = it->second;
    // Models nobody uses any more (e.g. after a scene switch): free them.
    if (group.entities.empty()) {
      unsigned int vbos[3] = {group.vbo_mat, group.vbo_idx, group.vbo_params};
      if (group.vbo_mat)
        glDeleteBuffers(3, vbos);
      it = instance_groups.erase(it);
      continue;
    }

    modelMatrices.clear();
    modelIndices.clear();
    modelParams.clear();
    for (auto &entity : group.entities) {
      auto transform = cm.get_component<Transform>(entity);
      if (transform != nullptr) {
        transform->update_camera(cam);
        update_transform(transform);
        modelMatrices.push_back(transform->model);
      }
      auto modelComp = cm.get_component<Model>(entity);
      modelIndices.push_back(modelComp->index);
      modelParams.push_back(modelComp->params);
    }
    group.shader = cm.get_component<Shader>(group.entities[0]);
    // An instance per entity with a transform; the others have nothing to
    // draw with.
    group.count = std::min(modelMatrices.size(), modelIndices.size());

    if (!group.vbo_mat) {
      unsigned int vbos[3];
      glGenBuffers(3, vbos);
      group.vbo_mat = vbos[0];
      group.vbo_idx = vbos[1];
      group.vbo_params = vbos[2];
    }
    size_t n = modelIndices.size();
    size_t grow = 0;
    if (n > group.capacity) {
      grow = std::max(n, group.capacity * 2);
      group.capacity = grow;
    }
    upload_instance_buffer(group.vbo_mat, modelMatrices.data(),
                           modelMatrices.size() * sizeof(glm::mat4),
                           grow * sizeof(glm::mat4));
    upload_instance_buffer(group.vbo_idx, modelIndices.data(),
                           modelIndices.size() * sizeof(float),
                           grow * sizeof(float));
    upload_instance_buffer(group.vbo_params, modelParams.data(),
                           modelParams.size() * sizeof(glm::vec4),
                           grow * sizeof(glm::vec4));
    ++it;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderingSystem::instanced_model_render(ComponentManager &cm, int x, int y,
                                             float zoom, int tick) {
  std::shared_ptr<Camera> cam = cm.get_component<Camera>(camera);
  if (cam != nullptr)
    pre_filter_lights(cam, x, y, zoom);

  const float time = (float)TE::now_seconds();
  glm::mat4 view{}, projection{};
  glm::vec3 cam_pos{};
  if (cam != nullptr) {
    view       = cam->get_view_matrix();
    camera_yaw = yaw_of(cam);
    projection = cam->get_projection_matrix(x, y, zoom);
    cam_pos    = cam->get_position();
  }

  static const std::string matrix_attribs[4] = {
      "aModelMatrix0", "aModelMatrix1", "aModelMatrix2", "aModelMatrix3"};

  for (auto &[key, group] : instance_groups) {
    auto &shader = group.shader;
    if (!shader || group.count == 0)
      continue;
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

    GLint matrix_locs[4];
    for (int i = 0; i < 4; ++i)
      matrix_locs[i] = shader->get_attrib(matrix_attribs[i]);
    GLint index_loc = shader->get_attrib("aModelIndex");
    GLint params_loc = shader->get_attrib("aModelParams");

    auto &modelPtr = group.model;
    for (auto &mesh : modelPtr->meshes) {
      glBindVertexArray(mesh->VAO);

      // Point the VAO at this group's instance buffers. The meshes' VAOs are
      // shared (other systems draw the same models), so this is done per draw.
      glBindBuffer(GL_ARRAY_BUFFER, group.vbo_mat);
      for (int i = 0; i < 4; ++i) {
        if (matrix_locs[i] >= 0) {
          glEnableVertexAttribArray(matrix_locs[i]);
          glVertexAttribPointer(matrix_locs[i], 4, GL_FLOAT, GL_FALSE,
                                sizeof(glm::mat4),
                                (void *)(sizeof(glm::vec4) * i));
          glVertexAttribDivisor(matrix_locs[i], 1);
        }
      }
      glBindBuffer(GL_ARRAY_BUFFER, group.vbo_idx);
      if (index_loc >= 0) {
        glEnableVertexAttribArray(index_loc);
        glVertexAttribPointer(index_loc, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                              (void *)0);
        glVertexAttribDivisor(index_loc, 1);
      }
      glBindBuffer(GL_ARRAY_BUFFER, group.vbo_params);
      if (params_loc >= 0) {
        glEnableVertexAttribArray(params_loc);
        glVertexAttribPointer(params_loc, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::vec4), (void *)0);
        glVertexAttribDivisor(params_loc, 1);
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
      glUniform1i(shader->get_uniform("materialKind"), mesh->kind);
      glUniform1i(shader->get_uniform("texture1"), 0);
      glDrawElementsInstanced(GL_TRIANGLES,
                              static_cast<unsigned int>(mesh->indices.size()),
                              GL_UNSIGNED_INT, 0, group.count);
      glBindVertexArray(0);
    }
  }
}
