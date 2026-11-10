#include "timeless/systems/cutscene_system.hpp"
#include "timeless/components/camera.hpp"
#include "timeless/components/skeletal_animation.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/core.hpp"
#include <cmath>

void CutsceneSystem::play(const Cutscene &cs) {
  cs_ = cs;
  playhead_ = 0.0f;
  prev_ = -1e-6f; // so triggers at t==0 fire on the first apply
  playing = true;
  apply(0.0f, prev_);
  prev_ = 0.0f;
}

void CutsceneSystem::finish() {
  if (!playing)
    return;
  float dur = cs_.duration();
  apply(dur, prev_);
  playhead_ = dur;
  prev_ = dur;
  playing = false;
  if (on_complete)
    on_complete();
}

void CutsceneSystem::update(ComponentManager & /*cm*/, float dt) {
  if (!playing)
    return;
  prev_ = playhead_;
  playhead_ += dt;

  float dur = cs_.duration();
  bool finished = false;
  float t = playhead_;

  if (t >= dur) {
    if (cs_.loop && dur > 0.0f) {
      // Wrap; re-fire triggers from the start of the new pass.
      playhead_ = std::fmod(playhead_, dur);
      prev_ = -1e-6f;
      t = playhead_;
    } else {
      t = dur;
      finished = true;
    }
  }

  apply(t, prev_);

  if (finished) {
    playing = false;
    if (on_complete)
      on_complete();
  }
}

void CutsceneSystem::apply(float t, float prev) {
  // Camera: sample position, aim at the sampled look-at point.
  if (!cs_.camera.position.empty()) {
    if (auto cam = TE::get_component<Camera>(find("camera"))) {
      glm::vec3 p = sample(cs_.camera.position, t);
      cam->set_position(p);
      if (!cs_.camera.lookAt.empty()) {
        glm::vec3 look = sample(cs_.camera.lookAt, t);
        cam->set_rotation(look_rotation(p, look, cam->get_base_forward(),
                                        cam->get_base_up()));
      }
    }
  }

  // Model transforms.
  for (const auto &tr : cs_.transforms) {
    auto tf = TE::get_component<Transform>(find(tr.target));
    if (!tf)
      continue;
    if (!tr.position.empty())
      tf->set_position(sample(tr.position, t));
    if (!tr.rotationEuler.empty())
      tf->setRotationEuler(sample(tr.rotationEuler, t));
    if (!tr.scale.empty())
      tf->set_scale(sample(tr.scale, t));
  }

  // Animation clip triggers crossed since the last frame.
  for (const auto &a : cs_.anims) {
    if (a.t > prev && a.t <= t) {
      if (auto sk = TE::get_component<SkeletalAnimation>(find(a.target))) {
        if (a.clip.empty())
          sk->playing = false;
        else {
          sk->setAnimation(a.clip);
          sk->playing = true;
        }
      }
    }
  }

  // Sound cues crossed since the last frame (same edge rule as anim triggers).
  for (const auto &s : cs_.sounds) {
    if (s.t > prev && s.t <= t && on_sound)
      on_sound(s.event);
  }

  // Report the currently-active dialogue line (empty clears it). Driven every
  // frame so it tracks the playhead and scrubs cleanly.
  if (on_dialogue)
    on_dialogue(cs_.dialogue_at(t));
}
