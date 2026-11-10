#pragma once
#include "timeless/components/component.hpp"
#include "timeless/components/skeletal_animation.hpp" // BoneKeyframes, SkeletalAnimationData
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Animates mesh node transforms directly from FBX object-level animations
// (i.e. non-skinned objects whose transforms are keyframed in Blender).
// Complementary to SkeletalAnimation which handles skinned bone hierarchies.
class NodeAnimation : public Component {
public:
  bool playing = true;
  std::unordered_map<std::string, SkeletalAnimationData> animations;
  std::shared_ptr<Model> model;

  NodeAnimation(std::shared_ptr<Model> model);

  void addAnimation(const std::string &name);

  void update(float dt);

private:
  std::vector<std::string> activeAnimations;
  std::unordered_map<std::string, float> activeTimes;

  void loadAnimations();

  glm::mat4 interpolate(const BoneKeyframes &keys, float time);
};
