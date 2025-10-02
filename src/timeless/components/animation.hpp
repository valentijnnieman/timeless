#pragma once
#include "component.hpp"
#include "glm/glm.hpp"
#include "timeless/entity.hpp"
#include "timeless/components/sprite.hpp"
#include <cmath>
#include <queue>

struct Bone {
    std::string name;
    std::shared_ptr<Quad> quad;
    std::shared_ptr<Transform> parent_transform;
    std::shared_ptr<Transform> transform;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Sprite> sprite;
    int sprite_index = 0;
};

struct Frame {
  float time;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

struct AnimationData {
  float duration;
  std::unordered_map<std::string, std::vector<Frame>> frames;
};


class Animation : public Component {
private:
  Entity entity;
  std::unordered_map<std::string, AnimationData> animations;
  std::string current_animation;
  float current_time = 0.0f;

public:
  std::vector<Bone> bones;

  void add_bone(const std::string &name, std::shared_ptr<Quad> quad, std::shared_ptr<Transform> transform, std::shared_ptr<Shader> shader,
                std::shared_ptr<Texture> texture, std::shared_ptr<Sprite> sprite, int sprite_index) {
      
    std::shared_ptr<Transform> bone_transform = std::make_shared<Transform>(*transform);
    bones.push_back({name, quad, transform, bone_transform, shader, texture, sprite, sprite_index});
  }
  void add_animation(const std::string& name, const AnimationData& data) {
      animations[name] = data;
  }
  void set_animation(const std::string& name) {
      if (animations.find(name) != animations.end()) {
          current_animation = name;
          current_time = 0.0f; // Reset time when changing animation
      }
  }

  void update(float dt) {
    current_time += (dt * 10000.0f);
    if (animations.find(current_animation) != animations.end()) {
      const auto& anim_data = animations.at(current_animation);

      if (current_time > anim_data.duration)
          current_time = fmod(current_time, anim_data.duration);

      for (auto& bone : bones) {
        if (anim_data.frames.find(bone.name) != anim_data.frames.end()) {
          const auto& keyframes = anim_data.frames.at(bone.name);
          if (current_time > anim_data.duration)
            bone.transform->position = bone.parent_transform->position;

          // Find the two keyframes surrounding current_time
          const Frame* prev = &keyframes.front();
          const Frame* next = &keyframes.back();
          for (size_t i = 1; i < keyframes.size(); ++i) {
              if (keyframes[i].time > current_time) {
                  prev = &keyframes[i - 1];
                  next = &keyframes[i];
                  break;
              }
          }

          // Interpolate between prev and next
          float t = (current_time - prev->time) / (next->time - prev->time);
          bone.transform->position = bone.parent_transform->position + glm::mix(prev->position, next->position, t);
          bone.transform->rot = bone.parent_transform->rotation + glm::radians(glm::mix(prev->rotation, next->rotation, t));
          bone.transform->scale = bone.parent_transform->scale + glm::mix(prev->scale,    next->scale,    t);
        }
        else {
          bone.transform->position = bone.parent_transform->position;
          bone.transform->rotation = bone.parent_transform->rotation;
          bone.transform->scale = bone.parent_transform->scale;
        }
      }
    }
  }
  // const Frame& get_current_frame() const {
  //     const auto& anim = animations.at(current_animation);
  //     size_t frame_index = static_cast<size_t>((current_time / anim.duration) * anim.frames.size()) % anim.frames.size();
  //     return anim.frames[frame_index];
  // }
};
