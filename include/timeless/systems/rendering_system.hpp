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

  unsigned int modelInstanceVBO;
  unsigned int modelInstanceVBO2;
  unsigned int modelInstanceVBO3;

  unsigned int attrLoc1;
  unsigned int attrLoc2;
  unsigned int attrLoc3;

  std::vector<glm::mat4> modelMatrices;
  std::vector<float> modelIndices;
  std::vector<glm::vec4> modelParams;

  std::vector<glm::mat4> models;
  std::vector<float> sprite_indices;
  std::vector<glm::vec2> sprite_sizes;

  std::vector<glm::vec3> filteredLightPositions;
  std::vector<glm::vec3> filteredLightColors;

public:
  Entity camera;

  Entity debug_ligth_ent;

  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightColors;
  float inst_jitter = 0.0f;
  float inst_jitter_speed = 0.0f;

  float ui_jitter = 0.0f;
  float ui_jitter_speed = 0.0f;

  float maxDistance = 2000.0f; // max distance for light calculations, beyond which light contribution is negligible

  void purge(ComponentManager &cm);
  void register_camera(Entity c);
  void update_transform(std::shared_ptr<Transform> transform,
                        glm::vec3 o = glm::vec3(0.0f));
  void set_shader_transform_uniforms(std::shared_ptr<Shader> shader,
                                     std::shared_ptr<Transform> transform,
                                     std::shared_ptr<Camera> camera,
                                     int x, int y, float zoom, int tick = 0);
  void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                  std::shared_ptr<Sprite> sprite, int tick = 0,
                                  glm::vec3 cam_position = glm::vec3(0.0));
  float get_text_width(Entity entity, ComponentManager &cm);
  float get_text_height(Entity entity, ComponentManager &cm);
  float get_text_font_height(Entity entity, ComponentManager &cm);
  void sort(ComponentManager &cm);
  void shadow_rotate(ComponentManager &cm);
  void shadow_reset_rotation(ComponentManager &cm);
  void pre_filter_lights(std::shared_ptr<Camera> cam);
  void calculate_lighting(Entity ent, ComponentManager &cm,
                          std::shared_ptr<Shader> shader,
                          std::shared_ptr<Camera> cam, int tick);
  void render(ComponentManager &cm, int x, int y, float zoom = 1.0,
              int tick = 0, float delta_time = 0.016f);
  bool is_in_frustum(const Frustum &frustum, const BoundingSphere &sphere);
  void init_instanced_buffers(std::shared_ptr<Quad> quad,
                              std::shared_ptr<Shader> shader,
                              size_t max_instances);
  void prepare_instanced(ComponentManager &cm, float x, float y, int zoom);
  void instanced_render(ComponentManager &cm, int x, int y,
                        std::shared_ptr<Quad> quad,
                        std::shared_ptr<Texture> texture,
                        std::shared_ptr<Shader> shader, float zoom = 1.0,
                        int tick = 0, glm::vec4 color = glm::vec4(1.0f));
  void init_instanced_model_buffers(std::shared_ptr<Model> model,
                                    std::shared_ptr<Shader> shader,
                                    size_t max_instances);
  void prepare_instanced_models(ComponentManager &cm,
                                std::shared_ptr<Shader> shader,
                                float x, float y, int zoom);
  // Prepare and render instanced models grouped by model pointer
  void instanced_model_render(ComponentManager &cm, int x, int y,
                              float zoom = 1.0, int tick = 0);
};
