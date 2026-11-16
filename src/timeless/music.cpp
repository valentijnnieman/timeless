#include "timeless/music.hpp"

#include "timeless/systems/sound_system.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include <json.hpp>

Score parse_score(const std::string &text, const std::string &name) {
  Score score;
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    std::cerr << "[TIMELESS] music " << name << ": empty, or not found" << std::endl;
    return score;
  }

  if (text[first] == '{') {
    const nlohmann::json parsed = nlohmann::json::parse(text, nullptr, false);
    if (parsed.is_discarded() || !parsed.contains("score") || !parsed["score"].is_array()) {
      std::cerr << "[TIMELESS] music " << name << ": not a score" << std::endl;
      return score;
    }
    // "synth" is what the key was called first.
    const std::string instrument =
        parsed.value("instrument", parsed.value("synth", std::string("StringSynth")));
    if (parsed.contains("instruments") && parsed["instruments"].is_object()) {
      for (const auto &[instrument_name, settings] : parsed["instruments"].items()) {
        if (!settings.is_object())
          continue;
        for (const auto &[parameter, value] : settings.items()) {
          std::string key = parameter;
          key[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[0])));
          score.instruments[instrument_name][key] = value.get<float>();
        }
      }
    }
    for (const auto &beat : parsed["score"]) {
      Beat notes;
      for (const auto &entry : beat) {
        ScoreNote note;
        note.name = entry.value("note", entry.value("event", std::string()));
        if (note.name.empty())
          continue;
        note.instrument = entry.value("instrument", entry.value("synth", instrument));
        note.at = std::clamp(entry.value("at", 0.0f), 0.0f, 1.0f);
        note.length = std::max(entry.value("length", 0.0f), 0.0f);
        note.velocity = std::clamp(entry.value("velocity", 1.0f), 0.0f, 1.0f);
        notes.push_back(note);
      }
      score.beats.push_back(std::move(notes));
    }
    return score;
  }

  std::istringstream lines(text);
  std::string line;
  while (std::getline(lines, line)) {
    std::istringstream names(line);
    Beat notes;
    std::string note_name;
    while (names >> note_name) {
      ScoreNote note;
      note.name = note_name;
      notes.push_back(note);
    }
    score.beats.push_back(std::move(notes));
  }
  return score;
}

Score loop_score(const Score &score, int ticks) {
  Score looped = score;
  while (!score.beats.empty() && static_cast<int>(looped.beats.size()) < ticks)
    looped.beats.insert(looped.beats.end(), score.beats.begin(), score.beats.end());
  return looped;
}

void apply_instruments(SoundSystem &sound, const Score &score) {
  for (const auto &[instrument, settings] : score.instruments)
    for (const auto &[parameter, value] : settings)
      sound.set_parameter(instrument, parameter, value);
}

void play_beat(SoundSystem &sound, const Beat &beat, bool reverse,
               float seconds_per_tick) {
  for (const ScoreNote &note : beat) {
    const float end = std::min(note.at + note.length, 1.0f);
    const float place = reverse ? 1.0f - end : note.at;
    const float delay = std::clamp(place, 0.0f, 1.0f) * seconds_per_tick;
    // The note's length is how long it rings; without one, the instrument decides.
    const float sustain = note.length > 0.0f ? note.length * seconds_per_tick : 0.95f;

    if (!sound.play_synth(note.name, note.instrument, note.velocity, sustain, delay, seconds_per_tick))
      sound.play_sample(note.name, delay, note.velocity);
  }
}
