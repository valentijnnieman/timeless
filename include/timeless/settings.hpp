#pragma once
#include <thread>
#include <glm/glm.hpp> // glm::vec4 used below
namespace TESettings
{
    inline glm::vec4 SCREEN_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Design / internal render resolution — fixed, never changed at runtime.
    // All game logic, UI positions and camera projections are in this space.
    inline int SCREEN_X = 1920;
    inline int SCREEN_Y = 1080;

    inline float VIEWPORT_SCALE = 1.0;
    inline int VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
    inline int VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
    inline float ZOOM = 1.0f;

    // The offscreen framebuffers the world is drawn through, as a fraction of
    // the viewport. Below 1 they're smaller than the design resolution (e.g.
    // the screen's own size, when that's smaller), which saves fill on a slow
    // GPU; layout, cameras and picking stay in design units. Pixel-sized
    // effects (the 1-bit dots, outlines) are then in these pixels.
    inline float RENDER_SCALE = 1.0f;
    inline int RENDER_X = VIEWPORT_X;
    inline int RENDER_Y = VIEWPORT_Y;

    // Actual OS window / framebuffer size.  Set this before TE::init() to
    // choose a different window size; it is updated automatically on resize.
    inline int WINDOW_X = SCREEN_X;
    inline int WINDOW_Y = SCREEN_Y;

    inline unsigned int SHADOW_WIDTH = 1024;
    inline unsigned int SHADOW_HEIGHT = 1024;

    inline bool FULLSCREEN = false;
    inline bool NATIVE_RESOLUTION = false;

    inline int MAX_THREADS = std::thread::hardware_concurrency();

    inline int MAX_TICKS = 200;

    // Physical WebGL drawing buffer size. On HiDPI displays (DPR>1) this is
    // larger than WINDOW_X/Y (e.g. 3840x2160 for a 1920x1080 CSS canvas on
    // macOS DPR=2). Use this for the final glViewport blit to the default
    // framebuffer. Use WINDOW_X/Y for mouse coordinate math instead.
    inline int DRAWING_BUFFER_X = SCREEN_X;
    inline int DRAWING_BUFFER_Y = SCREEN_Y;

    inline void set_viewport_scale(float new_scale) {
      VIEWPORT_SCALE = new_scale;
      VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
      VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
      RENDER_X = int(VIEWPORT_X * RENDER_SCALE + 0.5f);
      RENDER_Y = int(VIEWPORT_Y * RENDER_SCALE + 0.5f);
    }

    inline void set_render_scale(float new_scale) {
      RENDER_SCALE = new_scale;
      RENDER_X = int(VIEWPORT_X * RENDER_SCALE + 0.5f);
      RENDER_Y = int(VIEWPORT_Y * RENDER_SCALE + 0.5f);
    }

    // Only updates the window size; design resolution (SCREEN/VIEWPORT) stays fixed.
    inline void rescale_window(int width, int height)
    {
        WINDOW_X = width;
        WINDOW_Y = height;
    }
}
