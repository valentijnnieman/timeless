#pragma once

// ---------------------------------------------------------------------------
// timeless/systems/cutscene_system.hpp — plays back a single Cutscene.
//
// Manager-style System, mirroring SceneSystem: it owns the active cutscene and
// playhead directly (rather than iterating registered component entities), so
// only one cutscene runs at a time. Bind subject names to entities, then play():
//
//   auto cut = TE::get_system<CutsceneSystem>("CutsceneSystem");
//   cut->bind("camera", main_camera);
//   cut->bind("bod", beachbod.entity());
//   cut->on_complete = [&]{ resume_gameplay(); };
//   cut->play(cs);
//
// Drive it once per frame with update(cm, dt) (see beach/app.hpp). While
// `playing` is true the owning scene should defer its own camera / AI logic.
// ---------------------------------------------------------------------------

#include <functional>
#include <string>
#include <unordered_map>

#include "timeless/components/camera.hpp"
#include "timeless/components/skeletal_animation.hpp"
#include "timeless/components/transform.hpp"
#include "timeless/cutscene.hpp"
#include "timeless/entity.hpp"
#include "timeless/systems/system.hpp"
#include "timeless/timeless.hpp"

class CutsceneSystem : public System {
public:
  bool playing = false;
  // Fired once when a non-looping cutscene reaches its end (after the final
  // pose is applied). Use it to hand control back to gameplay or chain scenes.
  std::function<void()> on_complete;

  // Called every frame with the dialogue line active at the playhead ("" when
  // none). Rendering is the game's job (its own font / text rendering system),
  // which keeps this system font/shader-agnostic. Optional.
  std::function<void(const std::string &)> on_dialogue;

  CutsceneSystem() {}

  // Map a track's subject name (e.g. "camera", "bod") to an engine entity.
  void bind(const std::string &name, Entity e) { bound_[name] = e; }
  void clear_bindings() { bound_.clear(); }

  // Start playing `cs` from the beginning and immediately apply its first pose.
  void play(const Cutscene &cs) {
    cs_ = cs;
    playhead_ = 0.0f;
    prev_ = -1e-6f; // so triggers at t==0 fire on the first apply
    playing = true;
    apply(0.0f, prev_);
    prev_ = 0.0f;
  }

  void stop() { playing = false; }

  void update(ComponentManager & /*cm*/, float dt) {
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

private:
  Entity find(const std::string &name) const {
    auto it = bound_.find(name);
    return it == bound_.end() ? 0 : it->second;
  }

  // Set every controlled entity to its state at time t; fire any triggers in
  // the interval (prev, t].
  void apply(float t, float prev) {
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

    // Report the currently-active dialogue line (empty clears it). Driven every
    // frame so it tracks the playhead and scrubs cleanly.
    if (on_dialogue)
      on_dialogue(cs_.dialogue_at(t));
  }

  Cutscene cs_;
  float playhead_ = 0.0f;
  float prev_ = 0.0f;
  std::unordered_map<std::string, Entity> bound_;
};
