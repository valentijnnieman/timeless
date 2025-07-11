#pragma once
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"
#include <unordered_map>

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

	std::map<std::string, std::shared_ptr<FMOD::Studio::EventDescription>> event_descriptions;
public:
	float sampling_rate = 48000;
  void register_camera(Entity c)
  {
      camera = c;
  }
	void init()
	{
    if(fmodSystem == NULL) {
      FMOD_RESULT result = FMOD::Studio::System::create(&fmodSystem);
      if (result != FMOD_OK)
      {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
      }
      result = fmodSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
      if (result != FMOD_OK)
      {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
      }

      listener_atrbs = new FMOD_3D_ATTRIBUTES(FMOD_VECTOR(0,0,0), FMOD_VECTOR(0,0,0), FMOD_VECTOR(0, 0, 1), FMOD_VECTOR(0, 1, 0));
      atrbs = new FMOD_3D_ATTRIBUTES(FMOD_VECTOR(0,0,0), FMOD_VECTOR(0,0,0), FMOD_VECTOR(0, 0, 1), FMOD_VECTOR(0, 1, 0));

      std::cout << "[TIMELESS] SoundSystem initialized!" << std::endl;
    } else {
      std::cout << "[TIMELESS] SoundSystem was already initialized!" << std::endl;
    }
	}
	void load_bank_files(std::string master_bank_filename = "Assets/sound/Master.bank", std::string strings_bank_filename = "Assets/sound/Master.strings.bank")
	{
    if(masterBank == NULL) {
      FMOD_RESULT result = fmodSystem->loadBankFile(master_bank_filename.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
      if (result != FMOD_OK)
      {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
      }
    }
    if(stringsBank == NULL) {
      FMOD_RESULT result = fmodSystem->loadBankFile(strings_bank_filename.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);
      if (result != FMOD_OK)
      {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
      }
    }
		std::cout << "[TIMELESS] Sound banks loaded!" << std::endl;
	}
  void update(ComponentManager &cm) {
    auto main_camera = cm.get_component<Camera>(camera);
    if(main_camera != nullptr)
      listener_atrbs->position = FMOD_VECTOR(main_camera->get_position().x, main_camera->get_position().y, 0);
      fmodSystem->setListenerAttributes(0, listener_atrbs);

		fmodSystem->update();
	}
  void set_parameter(const std::string& name, int value) {
    fmodSystem->setParameterByName(name.c_str(), value);
  }
	void load_event_description(std::string soundevent_path)
	{
		//FMOD::Studio::EventDescription* new_event_description = NULL;
		//fmodSystem->getEvent(soundevent_path.c_str(), &new_event_description);
		//new_event_description->loadSampleData();
		//event_descriptions.insert({ soundevent_path, std::shared_ptr<FMOD::Studio::EventDescription>(new_event_description) });

		//std::cout << "[TIMELESS] event loaded!" << std::endl;
	}

	void trigger_event(std::string soundevent_path, float delay = 0.0f, bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0))
	{
    if(!events.contains(soundevent_path)){
      FMOD::Studio::EventDescription* new_event_description = NULL;
      std::string e = "event:/" + soundevent_path;
      fmodSystem->getEvent(e.c_str(), &new_event_description);

      new_event_description->loadSampleData();

      FMOD::Studio::EventInstance* event_instance = NULL;
      new_event_description->createInstance(&event_instance);

      events.insert({soundevent_path, event_instance});
    }

    auto event_instance = events.at(soundevent_path);

    if(spatial) {
      atrbs->position = FMOD_VECTOR(spatial_pos.x, spatial_pos.y, 0);
      event_instance->set3DAttributes(atrbs);
    }

		if (delay > 0)
		{
			event_instance->setProperty(
				FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY,
				delay
			);
		}

		event_instance->start();
		// event_instance->release();
	}

	void start_loop(std::string soundevent_path, float delay = 0.0f, bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0))
	{
    if(!is_looping) {
      FMOD::Studio::EventDescription* new_event_description = NULL;
      fmodSystem->getEvent(soundevent_path.c_str(), &new_event_description);

      new_event_description->loadSampleData();

      new_event_description->createInstance(&looping_event_instance);


      if(spatial) {
        atrbs->position = FMOD_VECTOR(spatial_pos.x, spatial_pos.y, 0);
        looping_event_instance->set3DAttributes(atrbs);
      }


      if (delay > 0)
      {
        looping_event_instance->setProperty(
          FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY,
          delay
        );
      }

      looping_event_instance->start();
      is_looping = true;
    }
	}
  void stop_looping_event()
  {
    if(is_looping) {
      looping_event_instance->stop(FMOD_STUDIO_STOP_MODE(0));
      looping_event_instance->release();
      is_looping = false;
    }
  }
	void unload()
	{
    for(auto& [event, instance] : events) {
      instance->release();
    }
		masterBank->unload();
		stringsBank->unload();
    delete(atrbs);
    delete(listener_atrbs);
		fmodSystem->release();
	}
	double half_to_time(int p, float speed)
	{
		 double half = ((sampling_rate * speed) * 2.0) / 2.0;

		 return p * half;
	}
	double quarter_to_time(int p, float speed)
	{
		 double quarter = ((sampling_rate * speed) * 2.0) / 4.0;

		 return p * quarter;
	}
	double eight_to_time(int p, float speed)
	{
		 double eight = ((sampling_rate * speed) * 2.0) / 8.0;

		 return p * eight;
	}
	double sixteenth_to_time(int p, float speed)
	{
		 double sixteenth = ((sampling_rate * speed) * 2.0) / 16.0;

		 return p * sixteenth;
	}
};
