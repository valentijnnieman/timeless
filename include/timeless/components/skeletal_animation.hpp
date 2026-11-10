#pragma once
#include "timeless/components/component.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Model;

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

class SkeletalAnimation : public Component {
public:
  std::string currentAnimation;
  float currentTime = 0.0f;
  bool playing = true;
  // Per-play loop control, set by setAnimation. Overrides the clip's own loop
  // flag so the same clip can be looped or played once depending on the caller.
  bool loop = true;
  // For one-shot (loop == false) clips: when true, drop the clip once it
  // finishes so the skeleton snaps back to the Blender rest (bind) pose instead
  // of freezing on the last frame.
  bool restWhenFinished = false;

  // Animation name -> AnimationData
  std::unordered_map<std::string, SkeletalAnimationData> animations;

  // Extra per-bone, local-space transforms applied on top of the current clip
  // (e.g. procedural lip/jaw motion). Keyed by bone name.
  std::unordered_map<std::string, glm::mat4> boneOverrides;

  void setBoneOverride(const std::string &name, const glm::mat4 &m) {
    boneOverrides[name] = m;
  }
  void clearBoneOverride(const std::string &name) { boneOverrides.erase(name); }

  // Computed pose matrices for skinning (one per bone, matches Model's bone
  // order)
  std::vector<glm::mat4> poseMatrices;

  // Reference to the entity's Model component
  std::shared_ptr<Model> model = nullptr;

  SkeletalAnimation(std::shared_ptr<Model> model);

  void setAnimation(const std::string &name, bool loop = true,
                    bool restWhenFinished = false);

<<<<<<< Updated upstream
  void stopAnimation() {
    playing = false;
    currentAnimation = "";
    std::fill(poseMatrices.begin(), poseMatrices.end(), glm::mat4(1.0f));
  }

  void update(float dt) {
    if (!playing || !model ||
        animations.find(currentAnimation) == animations.end())
      return;

    const SkeletalAnimationData &anim = animations.at(currentAnimation);
    currentTime += dt;
    if (loop && anim.duration > 0.0f) {
      currentTime = fmod(currentTime, anim.duration);
    } else if (currentTime > anim.duration) {
      currentTime = anim.duration;
      if (restWhenFinished) {
        // One-shot finished: clear the clip and stop advancing so the pose
        // below is computed with no active animation, i.e. the bind/rest pose.
        // We still fall through this frame to compute that pose once; later
        // frames early-return on !playing and hold the rest pose.
        currentAnimation = "";
        playing = false;
      }
    }

    poseMatrices.resize(model->boneInfos.size(), glm::mat4(1.0f));

    for (size_t i = 0; i < model->skeletonBones.size(); ++i) {
      if (model->skeletonBones[i].parentIndex == -1)
        computeBonePose(i, model->skeletonRootNodeTransform);
    }
  }
=======
  void stopAnimation();

  void update(float dt);

  // Bring poseMatrices up to date with the clip, if update() moved it on since
  // they were last computed. Call before reading them.
  void ensure_pose();
>>>>>>> Stashed changes

  void loadAnimations();

private:
  void computeBonePose(int boneIndex, const glm::mat4 &parentTransform);

  glm::mat4 getLocalTransform(const std::string &boneName,
                              const glm::mat4 &restLocal = glm::mat4(1.0f));

  glm::mat4 interpolate(const BoneKeyframes &keys, float time);
};
