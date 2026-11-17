#include "timeless/systems/sound_system.hpp"
#include "timeless/components/camera.hpp"
#include "timeless/core.hpp"
#include <tokkel/tokkel.hpp>
#include <tokkel/bass_synth.hpp>
#include <tokkel/bell_synth.hpp>
#include <tokkel/cello_synth.hpp>
#include <tokkel/drum_synth.hpp>
#include <tokkel/guitar_synth.hpp>
#include <tokkel/piano_synth.hpp>
#include <tokkel/sampler.hpp>
#include <tokkel/synth.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

// The engine's sampler with the game's sound events (add_sample()).
static const char *const sounds = "Sounds";

static tokkel::Vec3 to_tokkel(const glm::vec3 &v) { return {v.x, v.y, v.z}; }

// Out of line: tokkel::Engine is only complete in this file.
SoundSystem::SoundSystem() = default;
SoundSystem::~SoundSystem() = default;

void SoundSystem::register_camera(Entity c) { camera = c; }

void SoundSystem::init() {
    if (engine != nullptr) {
        std::cout << "[TIMELESS] SoundSystem was already initialized!" << std::endl;
        return;
    }
    tokkel::EngineConfig config;
    config.sample_rate = static_cast<std::uint32_t>(sampling_rate);
    engine = std::make_unique<tokkel::Engine>(config);
    if (!engine->ok()) {
        // No audio device (headless machine, CI): the game runs on, silent.
        std::cout << "[TIMELESS] SoundSystem: no audio device, running without sound"
                  << std::endl;
        return;
    }
    sampling_rate = static_cast<float>(engine->sample_rate());

    engine->add_sampler(sounds);
    engine->add_instrument("StringSynth", std::make_shared<tokkel::StringSynth>());
    engine->add_instrument("PianoSynth", std::make_shared<tokkel::PianoSynth>());
    engine->add_instrument("GuitarSynth", std::make_shared<tokkel::GuitarSynth>());
    engine->add_instrument("DrumSynth", std::make_shared<tokkel::DrumSynth>());
    engine->add_instrument("BassSynth", std::make_shared<tokkel::BassSynth>());
    engine->add_instrument("CelloSynth", std::make_shared<tokkel::CelloSynth>());
    engine->add_instrument("BellSynth", std::make_shared<tokkel::BellSynth>());
    apply_parameters(); // any set before a re-init
    std::cout << "[TIMELESS] SoundSystem initialized!" << std::endl;
}

bool SoundSystem::add_sample(const std::string &event, const std::string &audio_file_data) {
    auto sampler = engine != nullptr ? engine->sampler(sounds) : nullptr;
    if (sampler == nullptr)
        return false;
    if (!sampler->add(event, audio_file_data.data(), audio_file_data.size()))
        return false;
    sampled_events.insert(event);
    return true;
}

bool SoundSystem::has_samples(const std::string &event) const {
    return sampled_events.count(event) > 0;
}

void SoundSystem::update(ComponentManager &cm) {
    auto main_camera = cm.get_component<Camera>(camera);
    if (main_camera == nullptr)
        return;
    listener_position = main_camera->get_position();

    // The screen's axes in the world, from the view and projection the
    // renderer uses: a screen axis (in clip space, -1 to 1 across the view)
    // grows along its row of the matrix, and 1 over that row's length is how
    // far the view reaches along it. Whatever way the projection is mirrored.
    const glm::mat4 view_projection =
        main_camera->get_projection_matrix(TE::design_size().x, TE::design_size().y,
                                           TE::zoom()) *
        main_camera->get_view_matrix();
    const glm::vec3 across(view_projection[0][0], view_projection[1][0], view_projection[2][0]);
    const glm::vec3 up(view_projection[0][1], view_projection[1][1], view_projection[2][1]);
    const float width = glm::length(across), height = glm::length(up);
    if (width <= 0.0f || height <= 0.0f)
        return;
    // Orthographic: what doesn't move a point on screen is the view's depth.
    // (A perspective camera hears in plain 3D.)
    view_direction = main_camera->perspective ? glm::vec3(0.0f) : glm::cross(across, up);
    const float half_diagonal =
        std::sqrt(1.0f / (width * width) + 1.0f / (height * height));

    heard_near_distance = hearing_near * half_diagonal;
    heard_far_distance = hearing_far * half_diagonal;

    if (engine == nullptr)
        return;
    tokkel::Listener listener;
    listener.position = to_tokkel(heard_at(listener_position));
    listener.right = to_tokkel(across / width);
    listener.min_distance = heard_near_distance;
    listener.max_distance = heard_far_distance;
    engine->set_listener(listener);
}

glm::vec3 SoundSystem::heard_at(glm::vec3 position) const {
    const float length = glm::length(view_direction);
    if (length <= 0.0f)
        return position;
    const glm::vec3 depth = view_direction / length;
    return position - glm::dot(position, depth) * depth;
}

void SoundSystem::set_parameter(const std::string &category, const std::string &name,
                                float value) {
    parameters[{category, name}] = value;
    apply_parameter(category, name, value);
}

void SoundSystem::apply_parameters() {
    for (const auto &[key, value] : parameters)
        apply_parameter(key.first, key.second, value);
}

void SoundSystem::apply_parameter(const std::string &category, const std::string &name,
                                  float value) {
    if (engine == nullptr)
        return;
    if (category == "Main") {
        // The zoom's 2 to 40 (its target times 10) to the engine's 0 to 1.
        if (name == "Reverb")
            engine->set_reverb(value / 40.0f);
    } else if (category == "All") {
        for (const std::string &instrument : engine->instruments())
            engine->set_parameter(instrument, name, value);
    } else {
        engine->set_parameter(category, name, value);
    }
}

void SoundSystem::load_event_description(std::string) {}

bool SoundSystem::play_synth(const std::string &note, const std::string &instrument,
                             float amplitude, float sustain, float delay, float speed, bool tokkel) {
    const float frequency = tokkel::note_frequency(note);
    if (frequency <= 0.0f)
        return false;
    if (engine == nullptr || !engine->ok())
        return true;
    if (!engine->play_note(instrument, frequency, amplitude, sustain, delay, speed) &&
        unknown_instruments.insert(instrument).second)
        std::cerr << "[TIMELESS] SoundSystem: no instrument \"" << instrument
                  << "\", its notes are silent" << std::endl;
    return true;
}

bool SoundSystem::play_synth_at(const std::string &note, glm::vec3 position,
                                const std::string &instrument, float amplitude,
                                float sustain, float delay) {
    const float frequency = tokkel::note_frequency(note);
    if (frequency <= 0.0f)
        return false;
    if (engine == nullptr || !engine->ok())
        return true;
    if (!engine->play_note_at(instrument, to_tokkel(heard_at(position)), frequency, amplitude,
                              sustain, delay) &&
        unknown_instruments.insert(instrument).second)
        std::cerr << "[TIMELESS] SoundSystem: no instrument \"" << instrument
                  << "\", its notes are silent" << std::endl;
    return true;
}

float SoundSystem::heard_volume(glm::vec3 position) const {
    if (heard_far_distance <= 0.0f)
        return 1.0f;
    const float distance = glm::length(heard_at(position) - heard_at(listener_position));
    if (distance <= heard_near_distance)
        return 1.0f;
    if (distance >= heard_far_distance)
        return 0.0f;
    return (heard_far_distance - distance) / (heard_far_distance - heard_near_distance);
}

void SoundSystem::play_sample(const std::string &event, float delay, float volume) {
    if (engine != nullptr)
        engine->trigger(sounds, event, volume, false, delay);
}

void SoundSystem::trigger_event(std::string soundevent_path, bool spatial,
                                glm::vec3 spatial_pos) {
    // restart: a new trigger takes over from the event's previous one.
    if (engine == nullptr)
        return;
    if (spatial)
        engine->trigger_at(sounds, soundevent_path, to_tokkel(heard_at(spatial_pos)), 1.0f, true);
    else
        engine->trigger(sounds, soundevent_path, 1.0f, true);
}

void SoundSystem::preload_event(std::string soundevent_path) {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(_trigger_mutex);
#endif
    preloaded_events.insert(soundevent_path);
}

void SoundSystem::play_oneshot(std::string soundevent_path, bool spatial,
                               glm::vec3 spatial_pos) {
    if (engine == nullptr)
        return;
    if (spatial)
        engine->trigger_at(sounds, soundevent_path, to_tokkel(heard_at(spatial_pos)));
    else
        engine->trigger(sounds, soundevent_path);
}

void SoundSystem::start_loop(std::string soundevent_path, float, bool, glm::vec2,
                             std::string key) {
    if (key.empty())
        key = soundevent_path;
    looping_events.insert(key);
}

void SoundSystem::set_loop_position(const std::string &, glm::vec2) {}

void SoundSystem::stop_loop(const std::string &key) { looping_events.erase(key); }

void SoundSystem::stop_all_events() {
    looping_events.clear();
    if (engine != nullptr)
        engine->stop_events(sounds);
}

void SoundSystem::stop_event(std::string soundevent_path) {
    if (engine != nullptr)
        engine->stop_event(sounds, soundevent_path);
}

void SoundSystem::unload() {
    looping_events.clear();
    preloaded_events.clear();
    sampled_events.clear();
    unknown_instruments.clear();
    engine.reset(); // and its sampler and instruments
}

double SoundSystem::half_to_time(int p, float speed) {
    double half = ((sampling_rate * speed) * 2.0) / 2.0;
    return p * half;
}

double SoundSystem::quarter_to_time(int p, float speed) {
    double quarter = ((sampling_rate * speed) * 2.0) / 4.0;
    return p * quarter;
}

double SoundSystem::eight_to_time(int p, float speed) {
    double eight = ((sampling_rate * speed) * 2.0) / 8.0;
    return p * eight;
}

double SoundSystem::sixteenth_to_time(int p, float speed) {
    double sixteenth = ((sampling_rate * speed) * 2.0) / 16.0;
    return p * sixteenth;
}
