#pragma once
#include "timeless/components/component.hpp"
#include <glm/glm.hpp>
#include "timeless/timeless.hpp"
#include "timeless/components/sprite.hpp"
#include <cmath>
#include <queue>

struct Bone {
    Entity entity;
    std::string name;
    std::shared_ptr<Quad> quad;
    std::shared_ptr<Transform> parent_transform;
    std::shared_ptr<Transform> transform;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Sprite> sprite;
    int sprite_index = 0;
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
  std::vector<Bone> bones;
  Bone root;

  Animation(Entity entity, std::shared_ptr<Quad> quad, std::shared_ptr<Transform> transform, 
                std::shared_ptr<Texture> texture, std::shared_ptr<Sprite> sprite, int sprite_index) {

    std::shared_ptr<Transform> root_transform = std::make_shared<Transform>(*transform);
    root = Bone(entity, "root", quad, transform, root_transform, texture, sprite, sprite_index);
  }


  void add_bone(Entity entity, const std::string &name, std::shared_ptr<Quad> quad, std::shared_ptr<Transform> transform,
                std::shared_ptr<Texture> texture, std::shared_ptr<Sprite> sprite, int sprite_index) {
      
    std::shared_ptr<Transform> bone_transform = std::make_shared<Transform>(*transform);
    bones.push_back({entity, name, quad, transform, bone_transform, texture, sprite, sprite_index});
  }
  void add_animation(const std::string& name, const AnimationData& data) {
      animations[name] = data;
  }
  void set_animation(const std::string& name) {
      if (animations.find(name) != animations.end()) {
        reset();
        current_animation = name;
        current_time = 0.0f; // Reset time when changing animation
      }
  }

  void set_sprites_color(glm::vec4 color) {
      root.sprite->color = color;
      for (auto& bone : bones) {
        bone.sprite->color = color;
      }
  }

  void reset() {
      current_time = 0.0f;
      root.transform->position = root.parent_transform->position;
      root.transform->rotation = root.parent_transform->rotation;
      root.transform->scale = root.parent_transform->scale;
      for (auto& bone : bones) {
        bone.transform->position = bone.parent_transform->position;
        bone.transform->rotation = bone.parent_transform->rotation;
        bone.transform->scale = bone.parent_transform->scale;
      }
  }

  void update(float dt) {
    current_time += dt;
    if (animations.find(current_animation) != animations.end()) {
      const auto& anim_data = animations.at(current_animation);

      if (anim_data.loop) {
        if (current_time > anim_data.duration)
          current_time = fmod(current_time, anim_data.duration);
      } else {
        if (current_time > anim_data.duration)
        current_time = anim_data.duration;
      }

      // ANIMATE ROOT
      if (anim_data.frames.find(root.name) != anim_data.frames.end()) {
        const auto& keyframes = anim_data.frames.at(root.name);
        if (current_time > anim_data.duration)
          root.transform->position = root.parent_transform->position;

        // Find the two keyframes surrounding current_time
        const Keyframe* prev = &keyframes.front();
        const Keyframe* next = &keyframes.back();
        for (size_t i = 1; i < keyframes.size(); ++i) {
            if (keyframes[i].time > current_time) {
                prev = &keyframes[i - 1];
                next = &keyframes[i];
                break;
            }
        }

        // Interpolate between prev and next
        float t = (current_time - prev->time) / (next->time - prev->time);
        root.transform->position = root.parent_transform->position + glm::mix(prev->position, next->position, t);
        root.transform->setRotationEuler(root.parent_transform->getRotationEuler() + glm::mix(prev->rotation, next->rotation, t));
        root.transform->scale = root.parent_transform->scale + glm::mix(prev->scale,    next->scale,    t);
      }

      for (auto& bone : bones) {
        if (anim_data.frames.find(bone.name) != anim_data.frames.end()) {
          const auto& keyframes = anim_data.frames.at(bone.name);
          if (current_time > anim_data.duration)
            bone.transform->position = bone.parent_transform->position;

          // Find the two keyframes surrounding current_time
          const Keyframe* prev = &keyframes.front();
          const Keyframe* next = &keyframes.back();
          for (size_t i = 1; i < keyframes.size(); ++i) {
              if (keyframes[i].time > current_time) {
                  prev = &keyframes[i - 1];
                  next = &keyframes[i];
                  break;
              }
          }

          // Interpolate between prev and next
          float t = (current_time - prev->time) / (next->time - prev->time);
          bone.transform->position = root.transform->position + glm::mix(prev->position, next->position, t);
          bone.transform->setRotationEuler(root.transform->getRotationEuler() + glm::mix(prev->rotation, next->rotation, t));
          bone.transform->scale = root.transform->scale + glm::mix(prev->scale,    next->scale,    t);
        }
        else {
          bone.transform->position = root.transform->position;
          bone.transform->rotation = root.transform->rotation;
          bone.transform->scale = root.transform->scale;
        }
      }
    }
  }
};
