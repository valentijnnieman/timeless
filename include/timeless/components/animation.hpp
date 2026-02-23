#pragma once
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include "timeless/components/component.hpp"
#include <glm/glm.hpp>
#include "timeless/components/transform.hpp"
#include "timeless/components/quad.hpp"
#include "timeless/components/sprite.hpp"
#include "timeless/components/model.hpp"
#include "timeless/components/texture.hpp"
#include "timeless/entity.hpp"
#include <cmath>
#include <memory>
#include <unordered_map>
#include <queue>

struct Bone {
  Entity entity;
  std::string name;
  std::shared_ptr<Transform> parent_transform;
  std::shared_ptr<Transform> transform;
  virtual ~Bone() = default;

  Bone(Entity entity, const std::string &name,
       std::shared_ptr<Transform> parent_transform,
       std::shared_ptr<Transform> transform)
      : entity(entity), name(name), parent_transform(parent_transform),
        transform(transform) {}
};

struct SpriteBone : public Bone {
  std::shared_ptr<Quad> quad;
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Sprite> sprite;
  int sprite_index = 0;

  SpriteBone(Entity entity, const std::string &name, std::shared_ptr<Quad> quad,
             std::shared_ptr<Transform> parent_transform,
             std::shared_ptr<Transform> transform,
             std::shared_ptr<Texture> texture, std::shared_ptr<Sprite> sprite,
             int sprite_index)
      : Bone{entity, name, parent_transform, transform}, quad(quad),
        texture(texture), sprite(sprite), sprite_index(sprite_index) {}
};

struct ModelBone : public Bone {
  std::shared_ptr<Model> model;

  ModelBone(Entity entity, const std::string &name,
            std::shared_ptr<Transform> parent_transform,
            std::shared_ptr<Transform> transform, std::shared_ptr<Model> model)

      : Bone{entity, name, parent_transform, transform}, model(model) {}
};

struct Keyframe {
  float time;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

struct AnimationData {
  float duration;
  std::unordered_map<std::string, std::vector<Keyframe>> frames;
  bool loop = true;
};

class Animation : public Component {
private:
  std::unordered_map<std::string, AnimationData> animations;
  std::string current_animation;
  float current_time = 0.0f;

public:
  std::vector<std::shared_ptr<Bone>> bones;
  std::shared_ptr<Bone> root;
  bool playing = true;

  Animation(Entity entity, std::shared_ptr<Quad> quad,
            std::shared_ptr<Transform> transform,
            std::shared_ptr<Texture> texture, std::shared_ptr<Sprite> sprite,
            int sprite_index);

  Animation(Entity entity, std::shared_ptr<Transform> transform);

  Animation(Entity entity, std::shared_ptr<Transform> transform,
            std::shared_ptr<Model> model);

  void add_bone(Entity entity, const std::string &name,
                std::shared_ptr<Quad> quad,
                std::shared_ptr<Transform> transform,
                std::shared_ptr<Texture> texture,
                std::shared_ptr<Sprite> sprite, int sprite_index);
  void add_animation(const std::string &name, const AnimationData &data);
  void set_animation(const std::string &name);

  void stop_playing();

  void set_sprites_color(glm::vec4 color);

  void reset();

  void update(float dt);
};
