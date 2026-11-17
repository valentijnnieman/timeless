#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct aiScene;

// Keyframe data for a single bone
struct BoneKeyframes {
  std::vector<float> times;
  std::vector<glm::vec3> positions;
  std::vector<glm::quat> rotations;
  std::vector<glm::vec3> scales;
};

// Animation data for all bones
struct SkeletalAnimationData {
  float duration = 0.0f;
  bool loop = true;
  std::unordered_map<std::string, BoneKeyframes> boneKeyframes;
};

// A model's clips, by name.
using AnimationSet = std::unordered_map<std::string, SkeletalAnimationData>;

// Every clip in `scene`, converted from assimp's keyframes. nullptr without a
// scene. Immutable once built, so one set can be shared by everything that
// animates the same model (see Model::animation_set).
std::shared_ptr<const AnimationSet> load_animation_set(const aiScene *scene);
