#include <thread>
namespace TESettings
{
    static glm::vec4 SCREEN_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    static int SCREEN_X = 1920;
    static int SCREEN_Y = 1080;

    static float VIEWPORT_SCALE = 1.0;
    static int VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
    static int VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
    static float ZOOM = 1.0f;

    static unsigned int SHADOW_WIDTH = 1024;
    static unsigned int SHADOW_HEIGHT = 1024;

    static bool FULLSCREEN = false;

    static int MAX_THREADS = std::thread::hardware_concurrency();

    static int MAX_TICKS = 200;

    inline void set_viewport_scale(float new_scale) {
      VIEWPORT_SCALE = new_scale;
      VIEWPORT_X = SCREEN_X * VIEWPORT_SCALE;
      VIEWPORT_Y = SCREEN_Y * VIEWPORT_SCALE;
    }

    inline void rescale_window(int width, int height)
    {
        SCREEN_X = width;
        SCREEN_Y = height;
        // VIEWPORT_X = width * VIEWPORT_SCALE;
        // VIEWPORT_Y = height * VIEWPORT_SCALE;
    }
}
