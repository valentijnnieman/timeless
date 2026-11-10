#pragma once

// ---------------------------------------------------------------------------
// timeless/cutscene.hpp — engine-generic cutscene data + helpers.
//
// A Cutscene is a small, data-driven timeline used to script camera moves,
// character placement/orientation and animation triggers. It is played back by
// CutsceneSystem (see systems/cutscene_system.hpp). The data here knows nothing
// about any specific game: tracks target *bound names* (e.g. "camera", "bod")
// which the game maps to engine entities at play time, and everything operates
// on engine primitives (Camera, Transform, SkeletalAnimation).
//
// Two kinds of tracks:
//   * continuous (sampled at the playhead, scrub-friendly): camera pose +
//     look-at, and per-entity position / rotation / scale.
//   * triggers (fired once as the playhead crosses them): animation clips.
//
// Positions are world-space (cutscenes move things smoothly off any gameplay
// grid). Rotations are euler degrees (editor-friendly). Camera aim is authored
// as a look-at point; the orientation quaternion is derived (see look_rotation).
// ---------------------------------------------------------------------------

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Interpolation applied to the segment leading INTO a keyframe.
enum class Ease { Linear, Smoothstep };

inline const char *ease_name(Ease e) {
  return e == Ease::Smoothstep ? "smooth" : "linear";
}
inline Ease ease_from_string(const std::string &s) {
  return s == "smooth" ? Ease::Smoothstep : Ease::Linear;
}
inline float ease_apply(Ease e, float u) {
  switch (e) {
  case Ease::Smoothstep:
    return u * u * (3.0f - 2.0f * u);
  default:
    return u;
  }
}

struct Vec3Key {
  float t = 0.0f;
  glm::vec3 v{0.0f};
  Ease ease = Ease::Linear; // easing of the [prev -> this] segment
};

// A clip to play (or stop, if clip is empty) on a target when the playhead
// crosses time t.
struct AnimTrigger {
  float t = 0.0f;
  std::string target;
  std::string clip; // empty => stop the current animation
};

// A one-shot sound to fire when the playhead crosses time t. `event` is the
// game's sound-event name (e.g. an FMOD event path without the "event:/"
// prefix); playback is left to the game via CutsceneSystem::on_sound, so this
// stays audio-backend-agnostic. In a looping cutscene it re-fires each pass.
struct SoundCue {
  float t = 0.0f;
  std::string event;
};

// A line of dialogue / subtitle, shown while the playhead is within
// [t, t + duration). Rendering is left to the game (CutsceneSystem reports the
// active line via a callback) so this stays font/shader-agnostic. Keep text
// ASCII — the engine font only loads glyphs 0-127.
struct DialogueLine {
  float t = 0.0f;
  float duration = 2.0f;
  std::string text;
};

// Continuous control of one entity's transform over time. Empty channels are
// left untouched.
struct TransformTrack {
  std::string target;
  std::vector<Vec3Key> position;
  std::vector<Vec3Key> rotationEuler; // degrees
  std::vector<Vec3Key> scale;
};

// Continuous control of the camera: where it sits and what it looks at.
struct CameraTrack {
  std::vector<Vec3Key> position;
  std::vector<Vec3Key> lookAt;
};

struct Cutscene {
  CameraTrack camera;
  std::vector<TransformTrack> transforms;
  std::vector<AnimTrigger> anims;
  std::vector<SoundCue> sounds;
  std::vector<DialogueLine> dialogue;
  bool loop = false;
  float duration_override = 0.0f; // 0 => derive from the latest keyframe

  // Total length: the explicit override, or the latest key / trigger / dialogue
  // end time.
  float duration() const;

  // The dialogue line active at time t, or "" if none. Later lines win on
  // overlap. Evaluatable at any t, so it scrubs cleanly.
  std::string dialogue_at(float t) const;
};

// Sample a keyframe channel at time t. Holds the endpoints outside the range;
// eases each segment by its end key's ease. Returns 0 for an empty channel
// (callers guard with !empty()).
glm::vec3 sample(const std::vector<Vec3Key> &ks, float t);

// Build a camera orientation quaternion so that, given the camera's base
// forward/up axes (Camera::get_base_forward()/get_base_up()), get_forward()
// points from `pos` toward `target` exactly, with an approximately level
// horizon (world up defaults to +Z, beach's up axis).
//
// get_view_matrix() = lookAt(position, position + rotation*forward, rotation*up),
// so we need a rotation R with R*baseForward == normalize(target-pos). We map
// the camera's orthonormalized local frame onto the desired world frame.
glm::quat look_rotation(const glm::vec3 &pos, const glm::vec3 &target,
                        const glm::vec3 &baseForward, const glm::vec3 &baseUp,
                        const glm::vec3 &worldUp = glm::vec3(0, 0, 1));

// --- Text serialization ----------------------------------------------------
//
// # beach cutscene v1
// loop <0|1>
// duration <override or 0>
// cam_pos  <t> <x> <y> <z> <ease>
// cam_look <t> <x> <y> <z> <ease>
// track <name>            # following pos/rot/scale lines belong to this track
// pos   <t> <x> <y> <z> <ease>
// rot   <t> <x> <y> <z> <ease>   # euler degrees
// scale <t> <x> <y> <z> <ease>
// anim  <t> <target> <clip>
// sound <t> <event>              # fire a one-shot sound (game-defined event name)

bool save_cutscene(const Cutscene &cs, const std::string &path);

bool load_cutscene(Cutscene &cs, const std::string &path);
