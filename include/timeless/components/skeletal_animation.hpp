#pragma once
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "timeless/components/component.hpp"
#include "timeless/components/model.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <unordered_map>
#include <vector>

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

  // Animation name -> AnimationData
  std::unordered_map<std::string, SkeletalAnimationData> animations;

  // Computed pose matrices for skinning (one per bone, matches Model's bone
  // order)
  std::vector<glm::mat4> poseMatrices;

  // Reference to the entity's Model component
  std::shared_ptr<Model> model = nullptr;

  SkeletalAnimation(std::shared_ptr<Model> model) : model(model) {
    if (model) {
      poseMatrices.resize(model->boneInfos.size(), glm::mat4(1.0f));
      loadAnimations();
    }
  }

  void setAnimation(const std::string &name) {
    if (animations.find(name) != animations.end()) {
      currentAnimation = name;
      currentTime = 0.0f;
      playing = true;
    }
  }

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
    if (anim.loop && anim.duration > 0.0f)
      currentTime = fmod(currentTime, anim.duration);
    else if (currentTime > anim.duration)
      currentTime = anim.duration;

    poseMatrices.resize(model->boneInfos.size(), glm::mat4(1.0f));

    for (size_t i = 0; i < model->skeletonBones.size(); ++i) {
      if (model->skeletonBones[i].parentIndex == -1)
        computeBonePose(i, model->skeletonRootNodeTransform);
    }
  }

  void loadAnimations() {
    const aiScene *scene = model->scene;
    if (!scene) return;
    for (unsigned int animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx) {
      aiAnimation *aiAnim = scene->mAnimations[animIdx];
      SkeletalAnimationData animData;
      animData.duration =
          static_cast<float>(aiAnim->mDuration / aiAnim->mTicksPerSecond);
      animData.loop = true; // or set based on your needs

      // 2. Loop Through All Channels (one per animated bone)
      for (unsigned int channelIdx = 0; channelIdx < aiAnim->mNumChannels;
           ++channelIdx) {
        aiNodeAnim *channel = aiAnim->mChannels[channelIdx];
        std::string boneName(channel->mNodeName.C_Str());
        BoneKeyframes keyframes;

        // 3. Import Position Keyframes
        for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
          keyframes.times.push_back(static_cast<float>(
              channel->mPositionKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiVector3D pos = channel->mPositionKeys[i].mValue;
          keyframes.positions.emplace_back(pos.x, pos.y, pos.z);
        }

        // 4. Import Rotation Keyframes
        for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
          if (keyframes.times.size() <= i) // Ensure time vector is long enough
            keyframes.times.push_back(static_cast<float>(
                channel->mRotationKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiQuaternion q = channel->mRotationKeys[i].mValue;
          keyframes.rotations.emplace_back(q.w, q.x, q.y,
                                           q.z); // glm::quat(w, x, y, z)
        }

        // 5. Import Scale Keyframes
        for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
          if (keyframes.times.size() <= i)
            keyframes.times.push_back(static_cast<float>(
                channel->mScalingKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiVector3D s = channel->mScalingKeys[i].mValue;
          keyframes.scales.emplace_back(s.x, s.y, s.z);
        }

        // 6. Store keyframes for this bone
        animData.boneKeyframes[boneName] = keyframes;
      }

      // 7. Store the animation by name
      std::string animName = aiAnim->mName.length > 0
                                 ? aiAnim->mName.C_Str()
                                 : "Anim" + std::to_string(animIdx);
      animations[animName] = animData;
    }
  }

private:
  void computeBonePose(int boneIndex, const glm::mat4 &parentTransform) {
    const SkeletonBone &bone = model->skeletonBones[boneIndex];

    glm::mat4 localTransform = getLocalTransform(bone.name);
    glm::mat4 globalTransform = parentTransform * localTransform;

    auto it = model->boneMapping.find(bone.name);
    if (it != model->boneMapping.end()) {
      int mappingIdx = it->second;
      poseMatrices[mappingIdx] =
          globalTransform * model->boneInfos[mappingIdx].offsetMatrix;
    }

    for (int childIdx : bone.childrenIndices)
      computeBonePose(childIdx, globalTransform);
  }

  glm::mat4 getLocalTransform(const std::string &boneName) {
    auto animIt = animations.find(currentAnimation);
    if (animIt != animations.end()) {
      auto boneIt = animIt->second.boneKeyframes.find(boneName);
      if (boneIt != animIt->second.boneKeyframes.end() && !boneIt->second.times.empty())
        return interpolate(boneIt->second, currentTime);
    }
    return glm::mat4(1.0f);
  }

  glm::mat4 interpolate(const BoneKeyframes &keys, float time) {
    if (keys.positions.empty() || keys.rotations.empty() || keys.scales.empty())
      return glm::mat4(1.0f);

    size_t frame = 0;
    while (frame + 1 < keys.times.size() && keys.times[frame + 1] < time)
      ++frame;
    size_t nextFrame = std::min(frame + 1, keys.times.size() - 1);
    float t = 0.0f;
    if (keys.times[nextFrame] > keys.times[frame])
      t = (time - keys.times[frame]) / (keys.times[nextFrame] - keys.times[frame]);

    // Each channel may have fewer keyframes than times.size() (e.g. constant
    // scale exported as a single keyframe). Clamp per-vector to avoid OOB.
    size_t pf  = std::min(frame,     keys.positions.size() - 1);
    size_t pn  = std::min(nextFrame, keys.positions.size() - 1);
    size_t rf  = std::min(frame,     keys.rotations.size() - 1);
    size_t rn  = std::min(nextFrame, keys.rotations.size() - 1);
    size_t sf  = std::min(frame,     keys.scales.size() - 1);
    size_t sn  = std::min(nextFrame, keys.scales.size() - 1);

    glm::vec3 pos   = glm::mix(keys.positions[pf], keys.positions[pn], t);
    glm::quat rot   = glm::slerp(keys.rotations[rf], keys.rotations[rn], t);
    glm::vec3 scale = glm::mix(keys.scales[sf],     keys.scales[sn],     t);

    return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) *
           glm::scale(glm::mat4(1.0f), scale);
  }
};
