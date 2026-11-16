#pragma once
#include "timeless/systems/system.hpp"
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#ifndef __EMSCRIPTEN__
#include <mutex>
#endif

// The audio backend (vendor/tokkel). Only forward-declared, so including this
// header doesn't need tokkel's include path: just timeless-audio links it.
namespace tokkel {
class Engine;
}

// Sound through tokkel: recorded sounds by event name (trigger_event(), from
// the samples given to add_sample()) and notes on a synth (play_synth()).
// Loops aren't implemented yet: those calls only keep the bookkeeping their
// contracts describe.
//
// Spatial sounds (trigger_event() and play_oneshot() with `spatial`, and
// play_synth_at()) are heard from the registered camera, as they sit on
// screen: with an orthographic camera how deep into the view they are doesn't
// count. They come in at full volume within hearing_near of the middle of the
// view, fade out towards hearing_far and pan with their place left or right.
// Both are fractions of the view's half-diagonal, so they follow the zoom: at
// 1, a sound in a corner of the screen is just silent.
class SoundSystem : public System
{
private:
  std::unique_ptr<tokkel::Engine> engine;
  // The engine owns everything that sounds: init() adds its sampler (the
  // sound events of add_sample() and trigger_event()) and its instruments,
  // under the names play_synth() takes ("StringSynth", "PianoSynth",
  // "GuitarSynth", "DrumSynth", "BassSynth", "CelloSynth", "BellSynth"). To add an instrument, add it
  // there.
  // Names play_synth() was given that aren't instruments, reported once.
  std::unordered_set<std::string> unknown_instruments;

  // Keys of the loops that are playing, so several loops can play at once
  // (e.g. a UI loop plus one spatialised loop per walking character).
  std::unordered_set<std::string> looping_events;

  // Events that add_sample() gave a sample.
  std::unordered_set<std::string> sampled_events;

  // Events preloaded for play_oneshot().
  std::unordered_set<std::string> preloaded_events;

  // Every parameter as last set with set_parameter(), by (category, name).
  std::map<std::pair<std::string, std::string>, float> parameters;

  // Where the listener is: the registered camera's position, each update(),
  // and the direction it looks in, which spatial positions are flattened
  // along (see heard_at()).
  glm::vec3 listener_position = glm::vec3(0.0);
  glm::vec3 view_direction = glm::vec3(0.0);
  // hearing_near and hearing_far in world units, as update() last worked
  // them out from the view; 0 before the first update().
  float heard_near_distance = 0.0f;
  float heard_far_distance = 0.0f;
  Entity camera;

#ifndef __EMSCRIPTEN__
  std::mutex _trigger_mutex;
#endif

  void apply_parameter(const std::string &category, const std::string &name, float value);
  // A world position as the listener hears it: flattened onto the plane
  // through the world's origin facing the camera, for an orthographic camera
  // (whose view_direction is set), so only where it is on screen counts.
  glm::vec3 heard_at(glm::vec3 position) const;
public:
  SoundSystem();
  ~SoundSystem();

  float sampling_rate = 48000;
  // How far spatial sounds carry (see above), in the view's half-diagonals.
  float hearing_near = 0.15f;
  float hearing_far = 1.0f;
  void register_camera(Entity c);
  void init();
  // Give `event` a sample: the bytes of an audio file (wav, flac, mp3). Call
  // it again with the same event to add variations; trigger_event() then
  // plays a random one. False when there is no audio device or the data can't
  // be decoded. Call it after init().
  bool add_sample(const std::string &event, const std::string &audio_file_data);
  // Whether add_sample() gave `event` at least one sample.
  bool has_samples(const std::string &event) const;
  void update(ComponentManager &cm);
  // Set a sound parameter. It is kept, by category and name, and applied
  // again by apply_parameters() (init() does, so it outlasts an unload()).
  //   ("Main", "Reverb", v)        the main reverb on the whole mix: 0 none
  //                                to 40 the most (the zoom's range)
  //   ("All", name, v)             `name` on every instrument; each one that
  //                                knows it uses it, the others ignore it
  //   (instrument, name, v)        `name` on that instrument
  // What instruments know: every one "Reverb" (its own reverb, v / 20
  // seconds long, 0 none); the BassSynth also "Waveform" (0 sine, 1
  // triangle, 2 square, 3 saw). See tokkel's Synth::set_parameter().
  void set_parameter(const std::string &category, const std::string &name, float value);
  // Apply every kept parameter again.
  void apply_parameters();
  void load_event_description(std::string soundevent_path);
  // Play a note ("G2", "Bb3", "C#5") on an instrument: "StringSynth"
  // (plucked strings), "PianoSynth", "GuitarSynth", "BassSynth", "CelloSynth",
  // "BellSynth" (a decaying square wave) or
  // "DrumSynth" (C2 kick, D2 snare, F#2/A#2 closed/open hi-hat: General MIDI drum notes), starting `delay` seconds from now. An
  // unknown instrument is silent (reported once on stderr). False, and no
  // sound, when `note` isn't a note name, so a list of mixed names can try
  // this first and play the rest as samples.
  bool play_synth(const std::string &note, const std::string &instrument = "StringSynth",
                  float amplitude = 1.0f, float sustain = 0.95f, float delay = 0.0f, float speed = 1.0f, bool tokkel = false);
  // play_synth() from `position` in the world: quieter the further off it is
  // when it starts, silent out of hearing (instruments don't pan).
  bool play_synth_at(const std::string &note, glm::vec3 position,
                     const std::string &instrument = "StringSynth", float amplitude = 1.0f,
                     float sustain = 0.95f, float delay = 0.0f);
  // How loud a spatial sound from `position` would be heard, 0 to 1: 1
  // within hearing_near, 0 beyond hearing_far, linear in between. An
  // estimate for choosing or skipping sounds (the engine applies its own
  // falloff when they play); 1 before the first update().
  float heard_volume(glm::vec3 position) const;
  // Play one of the event's samples (see add_sample()). An event without
  // samples is silent. Triggering an event that is still playing restarts it:
  // the previous sound is cut off, as when FMOD restarted its one instance of
  // the event. With `spatial` it plays from `spatial_pos` in the world, and
  // stays there while the camera moves.
  void trigger_event(std::string soundevent_path, bool spatial = false,
                     glm::vec3 spatial_pos = glm::vec3(0.0));
  // One of the event's samples at `volume`, `delay` seconds from now, for
  // music: it doesn't restart the event, so hits overlap and ring out.
  void play_sample(const std::string &event, float delay = 0.0f, float volume = 1.0f);
  // Like trigger_event(), but every call is its own playback that runs to
  // its end, so copies of one event overlap.
  void play_oneshot(std::string soundevent_path, bool spatial = false,
                    glm::vec3 spatial_pos = glm::vec3(0.0));
  // Resolves an event and loads its sample data ahead of time so the first
  // play_oneshot() doesn't have to wait for it.
  void preload_event(std::string soundevent_path);
  // Start a looping playback of an event. `key` identifies the loop for
  // set_loop_position()/stop_loop() and defaults to the event path; pass an
  // explicit key to run several instances of the same event at once (e.g. one
  // per character). A no-op while a loop with this key is already playing.
  void start_loop(std::string soundevent_path, float delay = 0.0f,
                  bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0),
                  std::string key = "");
  // Move a playing spatial loop (no-op for unknown keys), so it can follow a
  // moving emitter each frame.
  void set_loop_position(const std::string &key, glm::vec2 spatial_pos);
  // Stop and release a playing loop (allowing fadeout); no-op for unknown keys.
  void stop_loop(const std::string &key);
  // Stop every playing event — loops and in-flight one-shots alike — e.g.
  // when skipping a cutscene or tearing down a scene. Loop
  // bookkeeping is cleared, so loops whose conditions still hold simply
  // restart on the next start_loop() call. Synth notes ring on.
  void stop_all_events();
  // Stop what the event plays (faded out in a few milliseconds), also its
  // triggers that haven't started yet.
  void stop_event(std::string soundevent_path);
  void unload();
  double half_to_time(int p, float speed);
  double quarter_to_time(int p, float speed);
  double eight_to_time(int p, float speed);
  double sixteenth_to_time(int p, float speed);
};
