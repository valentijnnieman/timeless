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

#include "timeless/cutscene.hpp"
#include "timeless/entity.hpp"
#include "timeless/systems/system.hpp"

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

  // Called with a sound-event name each time the playhead crosses a SoundCue.
  // Playback is the game's job (its own audio system), keeping this system
  // backend-agnostic. Optional.
  std::function<void(const std::string &)> on_sound;

  CutsceneSystem() {}

  // Map a track's subject name (e.g. "camera", "bod") to an engine entity.
  void bind(const std::string &name, Entity e) { bound_[name] = e; }
  void clear_bindings() { bound_.clear(); }

  // Start playing `cs` from the beginning and immediately apply its first pose.
  void play(const Cutscene &cs);

  void stop() { playing = false; }

  // Fast-forward to the end: snap every controlled entity to its final pose
  // (firing any triggers not yet crossed), then stop and fire on_complete —
  // exactly as if playback had reached the end naturally. Use this to skip a
  // cutscene without leaving actors frozen mid-motion. No-op if nothing plays.
  void finish();

  void update(ComponentManager & /*cm*/, float dt);

private:
  Entity find(const std::string &name) const {
    auto it = bound_.find(name);
    return it == bound_.end() ? 0 : it->second;
  }

  // Set every controlled entity to its state at time t; fire any triggers in
  // the interval (prev, t].
  void apply(float t, float prev);

  Cutscene cs_;
  float playhead_ = 0.0f;
  float prev_ = 0.0f;
  std::unordered_map<std::string, Entity> bound_;
};
