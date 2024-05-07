#pragma once
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

class SoundSystem : public System
{
private:
	FMOD::Studio::System* fmodSystem = NULL;
	FMOD::Studio::Bank* masterBank = NULL;
	FMOD::Studio::Bank* stringsBank = NULL;

	std::map<std::string, std::shared_ptr<FMOD::Studio::EventDescription>> event_descriptions;
public:
	float sampling_rate = 48000;
	void init()
	{
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
		std::cout << "[TIMELESS] SoundSystem initialized!" << std::endl;
	}
	void load_bank_files(std::string master_bank_filename = "Assets/sound/Master.bank", std::string strings_bank_filename = "Assets/sound/Master.strings.bank")
	{
		FMOD_RESULT result = fmodSystem->loadBankFile(master_bank_filename.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);
		if (result != FMOD_OK)
		{
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
		result = fmodSystem->loadBankFile(strings_bank_filename.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank);
		if (result != FMOD_OK)
		{
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
		std::cout << "[TIMELESS] Sound banks loaded!" << std::endl;
	}
	void update()
	{
		fmodSystem->update();
	}
	void load_event_description(std::string soundevent_path)
	{
		//FMOD::Studio::EventDescription* new_event_description = NULL;
		//fmodSystem->getEvent(soundevent_path.c_str(), &new_event_description);
		//new_event_description->loadSampleData();
		//event_descriptions.insert({ soundevent_path, std::shared_ptr<FMOD::Studio::EventDescription>(new_event_description) });

		//std::cout << "[TIMELESS] event loaded!" << std::endl;
	}

	void trigger_event(std::string soundevent_path, float delay = 0.0f)
	{
		FMOD::Studio::EventDescription* new_event_description = NULL;
		fmodSystem->getEvent(soundevent_path.c_str(), &new_event_description);

		new_event_description->loadSampleData();

		FMOD::Studio::EventInstance* event_instance = NULL;
		new_event_description->createInstance(&event_instance);


		if (delay > 0)
		{
			event_instance->setProperty(
				FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY,
				delay
			);
		}

		event_instance->start();
		std::cout << "[TIMELESS] event triggered!" << std::endl;
		event_instance->release();
	}
	void unload()
	{
		masterBank->unload();
		stringsBank->unload();
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
