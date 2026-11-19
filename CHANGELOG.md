# Changelog

All notable changes to Timeless. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/): below 1.0, a breaking change raises
the minor version (0.3 → 0.4), anything else the patch version.

Each version is tagged in git (`v0.4.0`) on `main` once it's merged.

## [Unreleased]

### Breaking

- **Unused shaders are removed from `shaders/`.** 42 shaders no game used are
  gone: the old non-WebGL set (`color.*`, `screen.*`, `sprite*.*`, `text.*`,
  ...) and unused WebGL ones (`webgl_decal.*`, `webgl_model_phong.*`,
  `webgl_model_water.*`, `webgl_sun.*`, `webgl_uibar*`, `webgl_uibutton.fs`,
  ...). A game that loads one of them needs its own copy.
- **`webgl_sobel.fs` and `webgl_screen_dithering.fs` are generic again.** The
  sobel pass works on the screen image, not the geometry pass's normals and
  depth, and the dithering pass no longer leaves marker pixels (alpha 0.5)
  undithered. Game-specific versions belong in the game's own shaders.

### Added

- **A `daylight` uniform.** `RenderingSystem::calculate_lighting()` also sends
  how much day it is (0 night, 1 day, from `sunriseTick`/`sunsetTick`), for
  shaders that light by it themselves: `lightColor` can't carry it for models,
  as `Model::render()` sets it to white for every mesh.

### Changed

- **tokkel's synths cost nothing when silent.** A synth computed every voice
  on every sample, playing or not (a `StringSynth` took 6–7% of a core either
  way). Now finished voices are skipped, and a synth silent for 0.2 s rests
  until its next note: a quiet instrument costs nothing, and a playing one far
  less. A reused `StringSynth` voice starts on clean strings, not on the last
  note's muted leftovers. Notes reach a synth's audio thread through a lock-free
  ring instead of a mutex.

## [0.4.1] - 2026-11-17

### Fixed

- **Typing text runs at the same speed at any frame rate.** A `Text` with a
  `type_speed` added at most one character per rendered frame, so a small
  `type_speed` typed at the frame rate: faster on high-refresh displays and in
  WASM than in a native build with a frame cap. Characters now appear on a
  clock (several in one frame on a slow display), and the typing callback runs
  once per frame that adds characters instead of once per character.

## [0.4.0] - 2026-11-17

### Breaking

- **The engine is an object.** `TE::init()` and `TE::cleanup()` are gone: make
  one `TE::Engine` and keep it alive while the game runs. It opens the window
  when it's made and closes it when it's destroyed, so declare it before
  anything holding OpenGL objects (shaders, textures, models), which then goes
  first. A second `Engine` aborts.

  ```cpp
  // before
  TE::init();
  // ...
  TE::cleanup();

  // after
  TE::EngineConfig config;
  config.native_resolution = true;
  auto engine = std::make_unique<TE::Engine>(config);
  ```

  The globals `TE::systems`, `TE::cm`, `TE::tm`, `TE::mis`, `TE::wm` and
  `TE::grid` are now members of the `Engine` (`TE::engine->cm`, ...); the
  `TE::get_*()` functions work as before.
- **`TESettings` and `timeless/settings.hpp` are gone.** What's fixed from
  startup is in `TE::EngineConfig` (read back with `TE::config()`); what changes
  while running belongs to what owns it, behind a `TE::` function:

  | Before | After |
  |---|---|
  | `TESettings::NATIVE_RESOLUTION`, `FULLSCREEN` | `EngineConfig::native_resolution`, `fullscreen` |
  | `TESettings::SCREEN_X/Y`, `VIEWPORT_X/Y` | `TE::design_size().x/y` (`EngineConfig::design_width/height`) |
  | `TESettings::RENDER_X/Y`, `RENDER_SCALE` | `TE::render_size().x/y`, `TE::render_scale()` (`EngineConfig::render_scale`) |
  | `TESettings::WINDOW_X/Y` | `TE::window_size().x/y` |
  | `TESettings::ZOOM` | `TE::zoom()`, `TE::set_zoom()` |
  | `TESettings::SCREEN_COLOR` | `TE::clear_color()`, `TE::set_clear_color()` |
  | `TESettings::MAX_TICKS` | the game's own; the sun's day length is `RenderingSystem::dayTicks` |
  | `set_viewport_scale()`, `set_render_scale()`, `rescale_window()` | gone: set the config, or let the window resize |

  `VIEWPORT_SCALE` (always 1), `SHADOW_WIDTH/HEIGHT`, `MAX_THREADS` and
  `DRAWING_BUFFER_X/Y` were unused and are gone.
- **`SceneSystem` owns its scenes.** `add_scene()` takes a
  `std::unique_ptr<Scene>` (and an optional name for `find_scene()`) and returns
  its index; `set_current_scene()` and `load_scene()` take an index;
  `get_current_scene()` returns a `Scene*`; `restart()` takes the fresh scene as
  a `unique_ptr`. `switch_to()` and `drop_other_scenes()` are gone.
- **Audio is on [tokkel](https://github.com/valentijnnieman/tokkel)**
  (miniaudio and STK) instead of FMOD, which no longer needs downloading.
  `SoundSystem` has no sound banks: `load_bank_files()` is gone, and an event's
  wav files are added with `add_sample()`. `trigger_event()` lost its `delay`
  and takes a `glm::vec3` position; `set_parameter()` takes a category, a name
  and a float.
- **Skeletal animation clips are shared.** A model's clips are converted once
  and shared by every `SkeletalAnimation` on it (`Model::animation_set`), so
  `SkeletalAnimation::animations` is a `std::shared_ptr<const AnimationSet>`:
  look a clip up with `clip(name)`. `BoneKeyframes` and `SkeletalAnimationData`
  moved to `timeless/components/animation_data.hpp`.
- `WindowManager`'s constructor takes the `EngineConfig`, and
  `add_framebuffer()`'s default size is `-1` (the render size).

### Added

- `TE::EngineConfig`: the window's size, native resolution, fullscreen, the
  design resolution and the render scale.
- Music (`timeless/music.hpp`): a score of one quarter note per tick, played a
  beat a tick and mirrored when time runs backward; `tools/midi_to_score.py` and
  `tools/score_to_midi.py` convert between MIDI and scores.
- `SoundSystem::play_synth()` / `play_synth_at()`: notes on named synthesized
  instruments, and `play_sample()`, `heard_volume()`.
- `Transform::hidden`: hides one entity's model, where `Model::hidden` hides it
  for every entity sharing the model.
- `RenderingSystem::dayTicks`, the day's length the sun crosses the sky over.
- `SceneSystem::find_scene()`, `scene_names()`, `current_index()`,
  `scene_count()`.
- Virtual destructors on `System` and `Event`, so deleting through the base is
  safe.

### Removed

- FMOD: `vendor/fmod/` and every build step that copied its libraries.

## 0.3 and earlier

Not recorded.
