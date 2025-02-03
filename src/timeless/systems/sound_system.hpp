#pragma once
#include "AL/al.h"
#include <AL/alc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

typedef struct WAVFILE {
  FILE *fptr;

  char riff[4];
  int file_size;
  char file_type_header[4];
  char fmt[4];
  int fmt_len;
  short audioformat;
  short channels;
  int sample_rate;
  int data_size;
  short byte_rate;
  short bits_per_sample;
  char data_header[4];

  int *data;
} WAVFILE;

WAVFILE *open_wav(const char *filename) {
  WAVFILE *new_wave = (WAVFILE *)malloc(1 * sizeof(WAVFILE));
  new_wave->fptr = fopen(filename, "r");

  if (new_wave->fptr== NULL) {
      perror("Error opening wav file");
  }

  int result = 0;
  result = fread(new_wave->riff, 4, 1, new_wave->fptr);
  result = fread(&new_wave->file_size, 4, 1, new_wave->fptr);
  result = fread(new_wave->file_type_header, 4, 1, new_wave->fptr);
  result = fread(new_wave->fmt, 4, 1, new_wave->fptr);
  result = fread(&new_wave->fmt_len, 4, 1, new_wave->fptr);
  result = fread(&new_wave->audioformat, 2, 1, new_wave->fptr);
  result = fread(&new_wave->channels, 2, 1, new_wave->fptr);
  result = fread(&new_wave->sample_rate, 4, 1, new_wave->fptr);
  result = fread(&new_wave->data_size, 4, 1, new_wave->fptr);
  result = fread(&new_wave->byte_rate, 2, 1, new_wave->fptr);
  result = fread(&new_wave->bits_per_sample, 2, 1, new_wave->fptr);
  result = fread(new_wave->data_header, 4, 1, new_wave->fptr);

  // get data now that wav header is retrieved
  int data[new_wave->data_size];
  result = fread(data, 4, new_wave->data_size, new_wave->fptr);
  fclose(new_wave->fptr);

  new_wave->data = &data[0];

  return new_wave;
}

class SoundSystem : public System
{
private:
  bool is_looping = false;
  ALCdevice *device;
  ALCcontext *context;

  ALuint source;

	Entity camera;

	std::map<std::string, std::string> event_descriptions;
	std::unordered_map<std::string, WAVFILE*> wavbank;
	std::unordered_map<std::string, ALuint> buffers;

public:
	float sampling_rate = 48000;
  void register_camera(Entity c)
  {
      camera = c;
  }
static void list_audio_devices(const ALCchar *devices)
{
        const ALCchar *device = devices, *next = devices + 1;
        size_t len = 0;

        fprintf(stdout, "Devices list:\n");
        fprintf(stdout, "----------\n");
        while (device && *device != '\0' && next && *next != '\0') {
                fprintf(stdout, "%s\n", device);
                len = strlen(device);
                device += (len + 1);
                next += (len + 2);
        }
        fprintf(stdout, "----------\n");
}
	void init()
	{
    list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
    device = alcOpenDevice(NULL);
    if(!device){
      std::cout << "[TIMELESS] Couldn't find sound device!" << std::endl;
    }

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context)) {
      std::cout << "[TIMELESS] Couldn't create sound context!" << std::endl;
    }

    alGenSources((ALuint)1, &source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    alGetError();
		std::cout << "[TIMELESS] SoundSystem initialized!" << std::endl;
	}
	void load_bank_files(std::string master_bank_filename = "Assets/sound/Master.bank", std::string strings_bank_filename = "Assets/sound/Master.strings.bank")
	{
	}
  void update(ComponentManager &cm) {
    // auto main_camera = cm.get_component<Camera>(camera);
    // if(main_camera != nullptr)
	}
  void set_parameter(const std::string& name, int value) {
  }
	void load_wav_file(std::string event, std::string filepath)
	{
    try {
      WAVFILE *wav = open_wav(filepath.c_str());
      // wavbank.insert({event, wav});
      buffers.insert({event, 0});
      alGenBuffers((ALuint)1, &buffers.at(event));

      alBufferData(buffers.at(event), wav->audioformat, wav->data, wav->data_size, wav->sample_rate);
    }
    catch(std::exception e){
      std::cout << "[TIMELESS] Couldn't load sound file!" << std::endl;
    }
	}

	void trigger_event(std::string soundevent_path, float delay = 0.0f, bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0))
	{
    std::cout << "Trigger event! " << std::endl;
    if (buffers.contains(soundevent_path)){
      std::cout << "wow we found it" << std::endl;
      alSourcei(source, AL_BUFFER, buffers.at(soundevent_path));
      alSourcePlay(source);
    }

	}

	void start_loop(std::string soundevent_path, float delay = 0.0f, bool spatial = false, glm::vec2 spatial_pos = glm::vec2(0.0))
	{
	}
  void stop_looping_event()
  {
  }
	void unload()
	{
    alDeleteSources(1, &source);
    for(auto& [event, buffer] : buffers) {
      alDeleteBuffers(1, &buffer);
    }
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
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
