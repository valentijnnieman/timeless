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
  FMOD::Studio::EventInstance* looping_event_instance = NULL;
  bool is_looping = false;

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
  void start_loop(std::string soundevent_path, float delay = 0.0f,
                  bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0));
  void stop_looping_event();
  void stop_event(std::string soundevent_path);
  void unload();
  double half_to_time(int p, float speed);
  double quarter_to_time(int p, float speed);
  double eight_to_time(int p, float speed);
  double sixteenth_to_time(int p, float speed);
};
