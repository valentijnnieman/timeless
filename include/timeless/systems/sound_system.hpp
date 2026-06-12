#pragma once
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "timeless/systems/system.hpp"
#include <glm/glm.hpp>
#include <map>
#include <unordered_map>
#ifndef __EMSCRIPTEN__
#include <mutex>
#endif

class SoundSystem : public System
{
private:
	FMOD::Studio::System* fmodSystem = NULL;
	FMOD::Studio::Bank* masterBank = NULL;
	FMOD::Studio::Bank* stringsBank = NULL;

  // Looping event instances, one per key, so several loops can play at once
  // (e.g. a UI loop plus one spatialised loop per walking character).
  std::unordered_map<std::string, FMOD::Studio::EventInstance*> looping_events;

  FMOD_3D_ATTRIBUTES* atrbs;
  FMOD_3D_ATTRIBUTES* listener_atrbs;
	Entity camera;

  std::unordered_map<std::string, FMOD::Studio::EventInstance*> events;

  // Cached event descriptions for play_oneshot(). FMOD owns these pointers; we
  // only cache them so repeated one-shots don't re-resolve the event each call.
  std::unordered_map<std::string, FMOD::Studio::EventDescription*> oneshot_descriptions;

	std::map<std::string, std::shared_ptr<FMOD::Studio::EventDescription>> event_descriptions;

#ifndef __EMSCRIPTEN__
  std::mutex _trigger_mutex;
#endif
public:
  float sampling_rate = 48000;
  void register_camera(Entity c);
  void init();
  void load_bank_files(std::string master_bank_filename = "Assets/sound/Master.bank",
                       std::string strings_bank_filename = "Assets/sound/Master.strings.bank");
  void update(ComponentManager &cm);
  void set_parameter(const std::string &name, int value);
  void load_event_description(std::string soundevent_path);
  void trigger_event(std::string soundevent_path, float delay = 0.0f,
                     bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0));
  // Fire-and-forget one-shot: creates a fresh event instance each call and
  // releases it when playback finishes, so multiple copies can overlap. Unlike
  // trigger_event(), it does not reuse a single cached instance.
  void play_oneshot(std::string soundevent_path, bool spatial = false,
                    glm::vec2 spatial_pos = glm::vec2(0.0));
  // Resolves an event and loads its sample data ahead of time so the first
  // play_oneshot() isn't silent while FMOD streams the sample in.
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
  // Stop every playing event — loops and in-flight one-shots alike — via the
  // master bus, e.g. when skipping a cutscene or tearing down a scene. Loop
  // bookkeeping is cleared, so loops whose conditions still hold simply
  // restart on the next start_loop() call.
  void stop_all_events();
  void stop_event(std::string soundevent_path);
  void unload();
  double half_to_time(int p, float speed);
  double quarter_to_time(int p, float speed);
  double eight_to_time(int p, float speed);
  double sixteenth_to_time(int p, float speed);
};
