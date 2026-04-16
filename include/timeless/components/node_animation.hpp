#pragma once
#include "assimp/scene.h"
#include "timeless/components/component.hpp"
#include "timeless/components/model.hpp"
#include "timeless/components/skeletal_animation.hpp" // BoneKeyframes, SkeletalAnimationData
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
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

  NodeAnimation(std::shared_ptr<Model> model) : model(model) {
    if (model) loadAnimations();
  }

  void addAnimation(const std::string &name) {
    if (animations.find(name) != animations.end()) {
      activeAnimations.push_back(name);
      activeTimes[name] = 0.0f;
    }
  }

  void update(float dt) {
    if (!playing || !model) return;

    for (const auto &name : activeAnimations) {
      const SkeletalAnimationData &anim = animations.at(name);
      float &t = activeTimes.at(name);
      t += dt;
      if (anim.loop && anim.duration > 0.0f)
        t = fmod(t, anim.duration);
      else if (t > anim.duration)
        t = anim.duration;

      for (const auto &[channelName, keys] : anim.boneKeyframes) {
        auto meshIt = model->meshNodeMap.find(channelName);
        if (meshIt == model->meshNodeMap.end()) continue;
        auto &mesh = model->meshes[meshIt->second];
        mesh->nodeTransform = mesh->nodeParentTransform * interpolate(keys, t);
      }
    }
  }

private:
  std::vector<std::string> activeAnimations;
  std::unordered_map<std::string, float> activeTimes;

  void loadAnimations() {
    const aiScene *scene = model->scene;
    if (!scene) return;
    for (unsigned int animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx) {
      aiAnimation *aiAnim = scene->mAnimations[animIdx];
      SkeletalAnimationData animData;
      animData.duration =
          static_cast<float>(aiAnim->mDuration / aiAnim->mTicksPerSecond);
      animData.loop = true;

      for (unsigned int ci = 0; ci < aiAnim->mNumChannels; ++ci) {
        aiNodeAnim *channel = aiAnim->mChannels[ci];
        std::string nodeName(channel->mNodeName.C_Str());
        BoneKeyframes keyframes;

        for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
          keyframes.times.push_back(static_cast<float>(
              channel->mPositionKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiVector3D p = channel->mPositionKeys[i].mValue;
          keyframes.positions.emplace_back(p.x, p.y, p.z);
        }
        for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
          if (keyframes.times.size() <= i)
            keyframes.times.push_back(static_cast<float>(
                channel->mRotationKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiQuaternion q = channel->mRotationKeys[i].mValue;
          keyframes.rotations.emplace_back(q.w, q.x, q.y, q.z);
        }
        for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
          if (keyframes.times.size() <= i)
            keyframes.times.push_back(static_cast<float>(
                channel->mScalingKeys[i].mTime / aiAnim->mTicksPerSecond));
          aiVector3D s = channel->mScalingKeys[i].mValue;
          keyframes.scales.emplace_back(s.x, s.y, s.z);
        }

        animData.boneKeyframes[nodeName] = keyframes;
      }

      std::string animName = aiAnim->mName.length > 0
                                 ? aiAnim->mName.C_Str()
                                 : "Anim" + std::to_string(animIdx);
      animations[animName] = animData;
    }
  }

  glm::mat4 interpolate(const BoneKeyframes &keys, float time) {
    size_t frame = 0;
    while (frame + 1 < keys.times.size() && keys.times[frame + 1] < time)
      ++frame;
    size_t nextFrame = std::min(frame + 1, keys.times.size() - 1);
    float t = 0.0f;
    if (keys.times[nextFrame] > keys.times[frame])
      t = (time - keys.times[frame]) /
          (keys.times[nextFrame] - keys.times[frame]);

    glm::vec3 pos   = glm::mix(keys.positions[frame], keys.positions[nextFrame], t);
    glm::quat rot   = glm::slerp(keys.rotations[frame], keys.rotations[nextFrame], t);
    glm::vec3 scale = glm::mix(keys.scales[frame], keys.scales[nextFrame], t);

    return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) *
           glm::scale(glm::mat4(1.0f), scale);
  }
};
