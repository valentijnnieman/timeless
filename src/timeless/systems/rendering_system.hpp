#pragma once
#include "../components/shader.hpp"
#include "../components/transform.hpp"
#include "system.hpp"
#include "timeless/managers/component_manager.hpp"
#include <string.h>

class RenderingSystem : public System {
public:
  Entity camera;

  void register_camera(Entity c) { camera = c; }

  void set_shader_transform_uniforms(std::shared_ptr<Shader> shader,
                                     std::shared_ptr<Transform> transform) {
    if (shader != nullptr && transform != nullptr) {
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1,
                         GL_FALSE, glm::value_ptr(transform->projection));
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE,
                         glm::value_ptr(transform->model));
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE,
                         glm::value_ptr(transform->view));
    }
  }
  void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                  std::shared_ptr<Sprite> sprite, int tick = 0,
                                  glm::vec3 cam_position = glm::vec3(0.0)) {
    if (shader != nullptr && sprite != nullptr) {
      glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
      glUniform1f(glGetUniformLocation(shader->ID, "index"), sprite->index);
      glUniform4fv(glGetUniformLocation(shader->ID, "highlightColor"), 1,
                   glm::value_ptr(sprite->color));
      glUniform2fv(glGetUniformLocation(shader->ID, "spriteSheetSize"), 1,
                   glm::value_ptr(sprite->spriteSheetSize));
      glUniform2fv(glGetUniformLocation(shader->ID, "spriteSize"), 1,
                   glm::value_ptr(sprite->spriteSize));
      glUniform1f(glGetUniformLocation(shader->ID, "time"), glfwGetTime());
      glUniform1f(glGetUniformLocation(shader->ID, "tick"), tick);
    }
  }

  void bind_text_values(Entity entity, ComponentManager &cm) {
    auto font = cm.get_font(entity);
    if (font != nullptr) {
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(font->VAO);
    }
  }

  float get_text_width(Entity entity, ComponentManager &cm) {
    auto font = cm.get_font(entity);
    auto text = cm.get_text(entity);

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
    auto font = cm.get_font(entity);
    auto text = cm.get_text(entity);

    float height = 0.0f;
    // Get width of string
    std::string::const_iterator c;
    for (c = text->text.begin(); c != text->text.end(); c++) {
      Glyph glyph = font->glyphs.at(*c);
      height = glyph.size.y;
    }
    return height;
  }

  void rotate_all_z(ComponentManager &cm, float z) {

    for (auto &entity : registered_entities) {
      auto transform = cm.get_transform(entity);
      if (transform != nullptr) {
        // transform->position.x += 42.0f;
        // transform->position.x += transform->width * 0.75f;
        // transform->position.y -= 8.0f;
        // transform->position.y += transform->height * 0.5f;
        transform->rotate(glm::vec3(0.0f, 0.0f, glm::radians(z)));
      }
    }
  }

  void reset_shadow(ComponentManager &cm, float z) {

    for (auto &entity : registered_entities) {
      auto transform = cm.get_transform(entity);
      if (transform != nullptr) {
        transform->rotate(glm::vec3(0.0f, 0.0f, glm::radians(z)));
        // transform->position.x -= 42.0f;
        // transform->position.x -= transform->width * 0.75f;
        // transform->position.y += 8.0f;
        // transform->position.y -= transform->height * 0.5f;
      }
    }
  }

  void sort(ComponentManager &cm) {

    std::stable_sort(registered_entities.begin(), registered_entities.end(),
                     [&](const Entity &a, const Entity &b) {
                       auto trans_a = cm.get_transform(a);
                       auto trans_b = cm.get_transform(b);
                       if (trans_a == nullptr || trans_b == nullptr)
                         return false;
                       return trans_a->position.z < trans_b->position.z;
                     });
  }

  void render(ComponentManager &cm, int x, int y, float zoom = 1.0,
              int tick = 0) {
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // get camera if set
    std::shared_ptr<Camera> cam = cm.get_camera(camera);

    float t_width = 256.0f;
    float t_height = 128.0f;

    // std::stable_sort(registered_entities.begin(),
    // registered_entities.end(),
    // [&, cm = cm](auto a, auto b) { 	auto trans_a =
    // cm.transforms.at(a); auto trans_b = cm.transforms.at(b);
    //
    // 	double a_h = trans_a->height / t_height;
    // 	double b_h = trans_b->height / t_height;
    //
    // 	double a_w = trans_a->width / t_width;
    // 	double b_w = trans_b->width / t_width;
    //
    // 	float a_x_pos = trans_a->grid_x;
    // 	float a_y_pos = trans_a->grid_y;
    // 	float b_x_pos = trans_b->grid_x;
    // 	float b_y_pos = trans_b->grid_y;
    //
    // 	return (a_y_pos + a_x_pos) < (b_y_pos + b_x_pos);
    // 	});
    //

    for (auto &entity : registered_entities) {
      auto shader = cm.get_shader(entity);
      if (shader != nullptr)
        shader->use();
      auto transform = cm.get_transform(entity);
      if (transform != nullptr) {
        transform->update(x, y, zoom);
        if (cam != nullptr) {
          transform->update_camera(cam->get_position());
        }
      }

      if (cm.texts.contains(entity)) {
        auto text = cm.get_text(entity);
        if (text->center) {
          cm.get_transform(entity)->update(
              x, y, 1.0f,
              glm::vec3(-get_text_width(entity, cm) * 0.5f,
                        -get_text_height(entity, cm) * 0.5f, 0.0f));
        }
        bind_text_values(entity, cm);
      }

      if (shader != nullptr && transform != nullptr)
        set_shader_transform_uniforms(cm.get_shader(entity),
                                      cm.get_transform(entity));
      if (cm.sprites.contains(entity))
        set_shader_sprite_uniforms(cm.get_shader(entity), cm.get_sprite(entity),
                                   tick, cam->get_position());
      if (cm.quads.contains(entity))
        cm.get_quad(entity)->render();
      if (cm.textures.contains(entity))
        cm.get_texture(entity)->render();

      if (cm.sprites.contains(entity)) {
        auto sprite = cm.get_sprite(entity);
        if (!sprite->hidden) {
          sprite->update();
          sprite->render();
        }
      }

      if (cm.texts.contains(entity)) {
        auto shader = cm.get_shader(entity);
        auto font = cm.get_font(entity);
        auto text = cm.get_text(entity);
        if (!text->hidden) {
          text->render(*font, 0.0f, 0.0f, get_text_height(entity, cm), shader);
        }
      }
    }
  }
};
