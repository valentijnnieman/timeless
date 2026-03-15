#include <thread>
namespace TESettings
{
    static glm::vec4 SCREEN_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Design / internal render resolution — fixed, never changed at runtime.
    // All game logic, UI positions and camera projections are in this space.
    static int SCREEN_X = 1920;
    static int SCREEN_Y = 1080;

    static float VIEWPORT_SCALE = 1.0;
    static int VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
    static int VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
    static float ZOOM = 1.0f;

    // Actual OS window / framebuffer size.  Set this before TE::init() to
    // choose a different window size; it is updated automatically on resize.
    static int WINDOW_X = SCREEN_X;
    static int WINDOW_Y = SCREEN_Y;

    static unsigned int SHADOW_WIDTH = 1024;
    static unsigned int SHADOW_HEIGHT = 1024;

    static bool FULLSCREEN = false;
    static bool NATIVE_RESOLUTION = false;

    static int MAX_THREADS = std::thread::hardware_concurrency();

    static int MAX_TICKS = 200;

    inline void set_viewport_scale(float new_scale) {
      VIEWPORT_SCALE = new_scale;
      VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
      VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
    }

    // Only updates the window size; design resolution (SCREEN/VIEWPORT) stays fixed.
    inline void rescale_window(int width, int height)
    {
        WINDOW_X = width;
        WINDOW_Y = height;
    }
}
