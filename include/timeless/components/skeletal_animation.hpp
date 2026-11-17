#pragma once
#include "timeless/components/animation_data.hpp" // BoneKeyframes, SkeletalAnimationData, AnimationSet
#include "timeless/components/component.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Model;

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

  // The model's clips (animation name -> data). Shared, read-only, by every
  // SkeletalAnimation of the same Model (see Model::animation_set): a crowd
  // of citizens on one model holds one copy, not one each. nullptr without a
  // model.
  std::shared_ptr<const AnimationSet> animations;

  // Extra per-bone, local-space transforms applied on top of the current clip
  // (e.g. procedural lip/jaw motion). Keyed by bone name.
  std::unordered_map<std::string, glm::mat4> boneOverrides;

  void setBoneOverride(const std::string &name, const glm::mat4 &m) {
    boneOverrides[name] = m;
  }
  void clearBoneOverride(const std::string &name) { boneOverrides.erase(name); }

  // Computed pose matrices for skinning (one per bone, matches Model's bone
  // order). Computed lazily: update() only advances the clip, and whoever
  // reads them calls ensure_pose() first, so entities nobody draws (off
  // screen) don't pay for posing their skeleton every frame.
  std::vector<glm::mat4> poseMatrices;
  bool pose_dirty = false;

  // Reference to the entity's Model component
  std::shared_ptr<Model> model = nullptr;

  SkeletalAnimation(std::shared_ptr<Model> model);

  void setAnimation(const std::string &name, bool loop = true,
                    bool restWhenFinished = false);

  void stopAnimation();

  void update(float dt);

  // Bring poseMatrices up to date with the clip, if update() moved it on since
  // they were last computed. Call before reading them.
  void ensure_pose();

  void loadAnimations();

private:
  // The clip called `name`, or nullptr if there's none.
  const SkeletalAnimationData *clip(const std::string &name) const;

  void computeBonePose(int boneIndex, const glm::mat4 &parentTransform);

  glm::mat4 getLocalTransform(const std::string &boneName,
                              const glm::mat4 &restLocal = glm::mat4(1.0f));

  glm::mat4 interpolate(const BoneKeyframes &keys, float time);
};
