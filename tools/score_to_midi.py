#!/usr/bin/env python3
"""Turn a JSON score (timeless/music.hpp) into a MIDI file, to edit it in a
notation program or DAW (MuseScore: File > Import), and convert it back with
midi_to_score.py.

    timeless/tools/score_to_midi.py Assets/music/song.json            # writes song.mid next to it
    timeless/tools/score_to_midi.py Assets/music/song.json -o ~/song.mid

Each instrument gets a track named after it, on the channel midi_to_score.py
reads back as that instrument (its INSTRUMENTS table; drums on channel 10),
with a General MIDI sound close to it, and its "volume" and "pan" from the
score's "instruments" as controllers 7 and 10. A tick of the score is a
quarter note at 60 BPM, a second as at the game's normal speed; a score with
"beats_per_tick" N (midi_to_score.py --beats-per-tick) has N quarter notes a
tick, at N * 60 BPM, so converting it back needs the same --beats-per-tick.
A note without a "length" lasts a tick. Sound events (samples, "Bloop") have no MIDI note and are left
out, with a count on stderr.
"""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import midi_to_score  # noqa: E402  (the channel map, note names, and the self-test's reader)

DIVISION = 480  # ticks per quarter note

# General MIDI programs (counted from 0) closest to each instrument.
PROGRAMS = {
    "StringSynth": 46,  # orchestral harp
    "GuitarSynth": 24,  # nylon guitar
    "BassSynth": 38,    # synth bass 1
    "CelloSynth": 42,   # cello
    "PianoSynth": 0,    # acoustic grand
}

NOTE_NUMBERS = {name: i for i, name in enumerate(midi_to_score.NOTE_NAMES)}
NOTE_NUMBERS.update({"Db": 1, "D#": 3, "Gb": 6, "Ab": 8, "A#": 10, "Cb": -1, "B#": 12,
                     "Fb": 4, "E#": 5})


def note_number(name):
    """The MIDI number of "C4", "Bb3", "F#2" (C4 = 60); None if it isn't a note."""
    if len(name) < 2 or not name[-1].isdigit():
        return None
    pitch, octave = name[:-1], int(name[-1])
    if pitch not in NOTE_NUMBERS:
        return None
    number = 12 * (octave + 1) + NOTE_NUMBERS[pitch]
    return number if 0 <= number <= 127 else None


def channel_of(instrument):
    """The channel (from 0) midi_to_score.py reads back as `instrument`."""
    if instrument == midi_to_score.DRUM_INSTRUMENT:
        return midi_to_score.DRUM_CHANNEL
    for channel, name in midi_to_score.INSTRUMENTS.items():
        if name == instrument:
            return channel - 1
    if instrument == midi_to_score.DEFAULT_INSTRUMENT:
        return 0
    return None


def _varlen(value):
    out = [value & 0x7F]
    value >>= 7
    while value:
        out.insert(0, (value & 0x7F) | 0x80)
        value >>= 7
    return bytes(out)


def _track(events):
    """A track chunk from (time, order, bytes) events, in time order."""
    body = b""
    now = 0
    for time, _, data in sorted(events, key=lambda e: (e[0], e[1])):
        body += _varlen(time - now) + data
        now = time
    body += _varlen(0) + bytes([0xFF, 0x2F, 0])
    return b"MTrk" + len(body).to_bytes(4, "big") + body


def _meta(kind, data):
    return bytes([0xFF, kind]) + _varlen(len(data)) + data


def to_midi(score):
    """The MIDI file (bytes) for a parsed JSON score, and a report."""
    default = score.get("instrument", score.get("synth", midi_to_score.DEFAULT_INSTRUMENT))
    settings = score.get("instruments", {})
    numerator, denominator = (int(x) for x in score.get("time_signature", "4/4").split("/"))
    per_tick = int(score.get("beats_per_tick", 1))
    ticks = DIVISION * per_tick  # MIDI ticks in a game tick

    tracks = {}  # instrument -> events
    report = {"notes": {}, "skipped_events": 0, "unknown": set()}
    for beat_index, beat in enumerate(score.get("score", [])):
        for entry in beat:
            if "note" not in entry:
                report["skipped_events"] += 1
                continue
            instrument = entry.get("instrument", entry.get("synth", default))
            channel = channel_of(instrument)
            number = note_number(entry["note"])
            if channel is None or number is None:
                report["unknown"].add(instrument if channel is None else entry["note"])
                continue
            start = round((beat_index + entry.get("at", 0.0)) * ticks)
            length = max(1, round((entry.get("length") or 1.0) * ticks))
            velocity = max(1, min(127, round(entry.get("velocity", 1.0) * 127)))
            events = tracks.setdefault(instrument, [])
            # Note-offs before note-ons at the same time, so repeated notes restart.
            events.append((start, 1, bytes([0x90 | channel, number, velocity])))
            events.append((start + length, 0, bytes([0x80 | channel, number, 0])))
            report["notes"][instrument] = report["notes"].get(instrument, 0) + 1

    conductor = [
        # A game tick is a second: 60 BPM, times the quarters in a tick.
        (0, 0, _meta(0x51, round(1_000_000 / per_tick).to_bytes(3, "big"))),
        (0, 0, _meta(0x58, bytes([numerator, max(0, denominator.bit_length() - 1), 24, 8]))),
    ]
    chunks = [_track(conductor)]
    # Pitched instruments in channel order, then the drums.
    for instrument in sorted(tracks, key=lambda name: channel_of(name)):
        channel = channel_of(instrument)
        setup = [(0, -2, _meta(0x03, instrument.encode()))]
        if instrument in PROGRAMS and channel != midi_to_score.DRUM_CHANNEL:
            setup.append((0, -1, bytes([0xC0 | channel, PROGRAMS[instrument]])))
        mixer = {key.lower(): value for key, value in settings.get(instrument, {}).items()}
        if "volume" in mixer:
            volume = max(0, min(127, round(mixer["volume"] * 100)))
            setup.append((0, -1, bytes([0xB0 | channel, 7, volume])))
        if "pan" in mixer:
            pan = max(0, min(127, round(64 + mixer["pan"] * 63)))
            setup.append((0, -1, bytes([0xB0 | channel, 10, pan])))
        chunks.append(_track(setup + tracks[instrument]))

    header = (1).to_bytes(2, "big") + len(chunks).to_bytes(2, "big") + DIVISION.to_bytes(2, "big")
    return b"MThd" + len(header).to_bytes(4, "big") + header + b"".join(chunks), report


def selftest():
    failures = 0

    def check(ok, what):
        nonlocal failures
        print(("ok  : " if ok else "FAIL: ") + what)
        failures += not ok

    score = {
        "time_signature": "3/4", "instrument": "StringSynth",
        "instruments": {"BassSynth": {"volume": 0.8, "pan": -0.5}},
        "score": [
            [{"note": "C2", "instrument": "DrumSynth", "velocity": 1.0},
             {"note": "E4", "at": 0.5, "length": 0.5, "velocity": 0.5}],
            [{"note": "A1", "instrument": "BassSynth", "length": 2.0}, {"event": "Bloop"}],
            [{"note": "Bb3", "instrument": "CelloSynth"}],
        ],
    }
    midi, report = to_midi(score)
    division, notes, end, signature = midi_to_score.read_midi(midi)
    check(division == DIVISION and signature == (3, 4), "writes the timing and time signature")
    found = {(start, length, channel, note) for start, length, channel, note, _ in notes}
    check((0, DIVISION, 9, 36) in found, "drums on channel 10, as General MIDI notes")
    check((DIVISION // 2, DIVISION // 2, 0, 64) in found, "a note keeps its place and length")
    check((DIVISION, 2 * DIVISION, 2, 33) in found, "an instrument on its channel")
    check(report["skipped_events"] == 1, "sound events are left out, and counted")
    data, _ = midi_to_score.to_json(division, notes, end, signature)
    back = {(i, n.get("note"), n.get("instrument", data["instrument"]))
            for i, beat in enumerate(data["score"]) for n in beat}
    check(back == {(0, "C2", "DrumSynth"), (0, "E4", "StringSynth"), (1, "A1", "BassSynth"),
                   (2, "Bb3", "CelloSynth")},
          "midi_to_score.py reads every note back on its instrument")
    check(bytes([0xB2, 7, 80]) in midi and bytes([0xB2, 10, 32]) in midi,
          "volume and pan as controllers 7 and 10")
    score["beats_per_tick"] = 2
    midi, _ = to_midi(score)
    division, notes, end, signature = midi_to_score.read_midi(midi)
    found = {(start, length, channel, note) for start, length, channel, note, _ in notes}
    check((DIVISION, DIVISION, 0, 64) in found and (2 * DIVISION, 4 * DIVISION, 2, 33) in found,
          "beats_per_tick: a tick is that many quarter notes")
    check(bytes([0xFF, 0x51, 3]) + (500_000).to_bytes(3, "big") in midi, "beats_per_tick 2: 120 BPM")
    data, _ = midi_to_score.to_json(division, notes, end, signature, beats_per_tick=2)
    back = {(i, n["note"], n["at"], n.get("length")) for i, beat in enumerate(data["score"]) for n in beat}
    check(back == {(0, "C2", 0.0, None), (0, "E4", 0.5, 0.5), (1, "A1", 0.0, 2.0), (2, "Bb3", 0.0, 1.0)},
          "midi_to_score.py --beats-per-tick 2 reads it back the same")
    check(note_number("C4") == 60 and note_number("Bb3") == 58 and note_number("C#5") == 73
          and note_number("Kick") is None, "reads note names")
    return failures == 0


def main():
    parser = argparse.ArgumentParser(description="Turn a JSON score into a MIDI file.")
    parser.add_argument("score", nargs="?", help="the .json score")
    parser.add_argument("-o", "--output", help="where to write it (default: next to the score, .mid)")
    parser.add_argument("--selftest", action="store_true", help="check the tool")
    args = parser.parse_args()
    if args.selftest:
        return 0 if selftest() else 1
    if not args.score:
        parser.error("give a .json score (or --selftest)")
    try:
        with open(args.score) as f:
            score = json.load(f)
    except (OSError, ValueError) as error:
        print(f"{args.score}: {error}", file=sys.stderr)
        return 1
    midi, report = to_midi(score)
    output = args.output or os.path.splitext(args.score)[0] + ".mid"
    with open(output, "wb") as f:
        f.write(midi)
    print(f"wrote {output}", file=sys.stderr)
    for instrument, count in sorted(report["notes"].items()):
        print(f"{count} notes on {instrument} (channel {channel_of(instrument) + 1})",
              file=sys.stderr)
    if report["skipped_events"]:
        print(f"{report['skipped_events']} sound events left out (no MIDI note)", file=sys.stderr)
    for name in sorted(report["unknown"]):
        print(f"left out: {name} (no channel for it in midi_to_score.INSTRUMENTS, or not a note)",
              file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
