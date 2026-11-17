#pragma once

namespace TE {

// How the engine starts up: everything the Engine (and the window it opens)
// needs to know before it exists. Passed to the Engine's constructor, so it
// can't be set "too late", and kept by the Engine, unchanged, for as long as
// it lives (TE::config()). What changes while running (the window's current
// size, the zoom, ...) isn't in here: see TE::window_size(), TE::zoom().
struct EngineConfig {
  // The window's size at startup, in screen pixels. Ignored with
  // native_resolution.
  int window_width = 1920;
  int window_height = 1080;
  // Open the window at the display's own resolution.
  bool native_resolution = false;
  // Borderless fullscreen on the desktop (WindowManager::set_fullscreen()
  // toggles it later).
  bool fullscreen = false;

  // The design resolution: the space all layout, cameras and picking work in,
  // whatever the window's size (TE::design_size()).
  int design_width = 1920;
  int design_height = 1080;
  // The offscreen framebuffers the world is drawn through, as a fraction of
  // the design resolution. Below 1 they're smaller (e.g. a screen with fewer
  // pixels than the design), which saves fill on a slow GPU; pixel-sized
  // effects (the 1-bit dots, outlines) are then in these pixels.
  float render_scale = 1.0f;
};

} // namespace TE
