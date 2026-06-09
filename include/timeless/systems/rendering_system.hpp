#pragma once
#include "timeless/components/animation.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/shader.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/managers/component_manager.hpp"
#include "timeless/systems/system.hpp"
#include <memory>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

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

  // Directional light (sun). lightPos is the direction the light points toward
  // (shader uses normalize(-lightPos) as L).
  glm::vec3 dirLightPos   = {1.0f, 0.0f, 1.0f};
  glm::vec3 dirLightColor = {1.0f, 1.0f, 1.0f};

  // Override ambient strength. When >= 0 this value is used directly instead
  // of the day-cycle formula (cos(angle)*0.5+0.1).
  float ambientStrength = -1.0f;

  // Per-entity sun-burn, applied as shader uniforms around the model draw, so
  // entities sharing one Model burn individually. xyz = burn colour (albedo
  // multiply target), w = amount 0..1. The fragment shader reddens only sunlit
  // fragments (shadowed parts are spared). Absent entities render unburnt.
  std::unordered_map<Entity, glm::vec4> entity_burn;
  void set_entity_burn(Entity e, glm::vec3 color, float amount) {
    entity_burn[e] = glm::vec4(color, amount);
  }
  void clear_entity_burn(Entity e) { entity_burn.erase(e); }

  // Per-entity sun-tan — same mechanism as burn (xyz = tan colour, w = amount).
  // Tan and burn stack in the shader; both only show on sunlit fragments.
  std::unordered_map<Entity, glm::vec4> entity_tan;
  void set_entity_tan(Entity e, glm::vec3 color, float amount) {
    entity_tan[e] = glm::vec4(color, amount);
  }
  void clear_entity_tan(Entity e) { entity_tan.erase(e); }

  // Per-entity "sheen" (0..1) — a freshly-oiled gloss + selection rim, used to
  // show which bod is being lotioned. Same per-draw-uniform mechanism as burn.
  std::unordered_map<Entity, float> entity_sheen;
  void set_entity_sheen(Entity e, float v) { entity_sheen[e] = v; }
  void clear_entity_sheen(Entity e) { entity_sheen.erase(e); }

  // Per-entity alpha (0..1) for translucent models — the shader outputs this as
  // the fragment alpha and the renderer drops depth writes for translucent
  // entities so they don't occlude geometry behind them (e.g. an umbrella faded
  // to reveal the bod underneath). Absent entities render fully opaque.
  std::unordered_map<Entity, float> entity_alpha;
  void set_entity_alpha(Entity e, float v) { entity_alpha[e] = v; }
  void clear_entity_alpha(Entity e) { entity_alpha.erase(e); }

  // Entities that should not cast directional shadows (skipped in the depth
  // pass) — e.g. a sky sun placed near the light, which otherwise drops a stray
  // shadow on the beach.
  std::unordered_set<Entity> no_shadow_cast;
  void set_casts_shadow(Entity e, bool casts) {
    if (casts)
      no_shadow_cast.erase(e);
    else
      no_shadow_cast.insert(e);
  }

  // Shadow mapping (directional light only)
  unsigned int shadowFBO = 0;
  unsigned int shadowDepthTex = 0;
  int shadowMapSize = 2048;
  // Light-space ortho half-extent + centre for the directional shadow map.
  // Smaller ortho = sharper shadows (less world per texel) but covers less area.
  // The default fits a very large scene; set it to your scene's extent to make
  // the shadow-map resolution actually count.
  float shadowOrthoSize = 3000.0f;
  glm::vec3 shadowCenter = glm::vec3(0.0f, -200.0f, 0.0f);
  glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
  std::shared_ptr<Shader> shadowDepthShader;

  void setup_shadow_map();
  glm::mat4 compute_light_space_matrix() const;
  void render_shadow_pass(ComponentManager &cm, std::shared_ptr<Camera> cam,
                          float delta_time);

  void purge(ComponentManager &cm);
  void register_camera(Entity c);
  void update_transform(std::shared_ptr<Transform> transform,
                        glm::vec3 o = glm::vec3(0.0f));
  void set_shader_transform_uniforms(std::shared_ptr<Shader> shader,
                                     std::shared_ptr<Transform> transform,
                                     const glm::mat4& view,
                                     const glm::mat4& projection,
                                     float time,
                                     int tick = 0);
  void set_shader_sprite_uniforms(std::shared_ptr<Shader> shader,
                                  std::shared_ptr<Sprite> sprite,
                                  float time,
                                  int tick = 0,
                                  glm::vec3 cam_position = glm::vec3(0.0));
  float get_text_width(Entity entity, ComponentManager &cm);
  float get_text_height(Entity entity, ComponentManager &cm);
  float get_text_font_height(Entity entity, ComponentManager &cm);
  void sort(ComponentManager &cm);
  void shadow_rotate(ComponentManager &cm);
  void shadow_reset_rotation(ComponentManager &cm);
  void pre_filter_lights(std::shared_ptr<Camera> cam);
  void calculate_lighting(std::shared_ptr<Shader> shader,
                          const glm::vec3& cam_pos, int tick);
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
  bool upload_bone_matrices(Entity entity, ComponentManager &cm,
                            std::shared_ptr<Shader> shader);
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
