#pragma once
#include "system.hpp"

class AnimationSystem : public System {
public:
  AnimationSystem(){};

  void update(ComponentManager &cm) {
    for (auto entity : registered_entities) {
      auto animation = cm.get_component<Animation>(entity);
      if (animation != nullptr) {
        if (!animation->positions.empty()) {
          auto transform = cm.get_component<Transform>(entity);
          if (transform != nullptr) {
            glm::vec3 dir = animation->positions.front();
            transform->set_position(dir);
            animation->positions.pop();
            if (animation->loop) {
              animation->positions.push(dir);
            }
          }
        }
        if (!animation->scales.empty()) {
          auto transform = cm.get_component<Transform>(entity);
          if (transform != nullptr) {
            glm::vec3 scale = animation->scales.front();
            transform->set_scale(scale);
            animation->scales.pop();
          }
        }

        if (!animation->rotations.empty()) {
          auto transform = cm.get_component<Transform>(entity);
          if (transform != nullptr) {
            glm::vec3 euler_angles = animation->rotations.front();
            transform->rotate(euler_angles);
            animation->rotations.pop();
            if (animation->loop) {
              animation->rotations.push(euler_angles);
            }
          }
        }

        if (!animation->opacities.empty()) {
          auto sprite = cm.get_component<Sprite>(entity);
          if(sprite != nullptr) {
            float o = animation->opacities.front();
            sprite->color.a = o;
            animation->opacities.pop();
          }

            auto text = cm.get_component<Text>(entity);
            if(text != nullptr) {
              float o = animation->opacities.front();
              // text->color.a = o;
              for (auto &c : text->color_vector) {
                c.a = o;
              }
              animation->opacities.pop();
            }
        }
      }
    }
  }
};
