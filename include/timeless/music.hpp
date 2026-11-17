#pragma once
#include <map>
#include <string>
#include <vector>

class SoundSystem;

// Music as a score of one quarter note per game tick, so it plays a beat a
// tick: faster with a faster game, and backwards when it rewinds. Scores are
// made from MIDI files with timeless/tools/midi_to_score.py.
//
// The game reads the file (from disk, a packed asset, wherever) and hands
// over its text, then plays one beat from each tick:
//
//   const Score music = loop_score(parse_score(text, "song.json"), ticks);
//   apply_instruments(sound, music);
//   ... in tick i's instruction:
//   play_beat(sound, music.beats[i], reverse, seconds_per_tick);

// One note of a beat: a note name ("G3") played on `instrument` (a name
// SoundSystem::play_synth() knows: "StringSynth", "PianoSynth", "GuitarSynth",
// "BassSynth", "CelloSynth", or "DrumSynth", whose notes are drums: C2 kick, D2 snare, F#2/A#2 closed/open hi-hat), or a
// sound event ("Bloop") played as a sample.
struct ScoreNote {
  std::string name;
  std::string instrument = "StringSynth";
  float at = 0.0f;       // where in the beat it starts, 0 to 1
  float length = 0.0f;   // in beats; 0 leaves it to the instrument (its default sustain)
  float velocity = 1.0f; // 0 to 1
};
using Beat = std::vector<ScoreNote>;

struct Score {
  std::vector<Beat> beats;
  // How each instrument is set up before it plays: parameter names (as
  // SoundSystem::set_parameter() takes them, "Volume", "Pan", "Reverb", ...)
  // to values, per instrument name.
  std::map<std::string, std::map<std::string, float>> instruments;
};

// The score in `text`, a file's contents: JSON from midi_to_score.py
// (recognised by its opening "{"), or the older text format (one line of names
// per beat, every note on the beat, all on the StringSynth). Empty, with a
// message on stderr naming `name`, when it isn't a score.
//
// In JSON, a note's "instrument" picks what plays it; without one, the
// score's own "instrument" does, and without that the StringSynth.
// "instruments" sets them up: each key is a parameter of that instrument,
// its first letter made upper case ("volume" is "Volume"), so anything the
// instrument knows can be set there:
//
//   {"instrument": "PianoSynth",
//    "instruments": {"PianoSynth": {"volume": 0.8, "pan": -0.3, "reverb": 20},
//                    "BassSynth": {"waveform": 2}},
//    "score": [
//     [{"note": "D3"}, {"note": "A4", "instrument": "StringSynth", "at": 0.5}],
//     [{"note": "C2", "instrument": "DrumSynth"}]]}
Score parse_score(const std::string &text, const std::string &name = "score");

// The score's beats repeated until there are at least `ticks`.
Score loop_score(const Score &score, int ticks);

// Set up the instruments as the score's "instruments" says, through
// SoundSystem::set_parameter() (so the settings are kept).
void apply_instruments(SoundSystem &sound, const Score &score);

// Play one beat over a tick of `seconds_per_tick`. Each note starts as far
// into the tick as it is into the beat; played `reverse`, the beat is
// mirrored, so a note ends where it started and the beat runs backwards.
void play_beat(SoundSystem &sound, const Beat &beat, bool reverse,
               float seconds_per_tick);
