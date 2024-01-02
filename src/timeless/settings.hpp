
namespace TESettings
{
    static int SCREEN_X = 1920;
    static int SCREEN_Y = 1080;

    static int VIEWPORT_SCALE = 1;
    static int VIEWPORT_X = 1920 * VIEWPORT_SCALE;
    static int VIEWPORT_Y = 1080 * VIEWPORT_SCALE;

    void rescale_window(int width, int height)
    {
        SCREEN_X = width;
        SCREEN_Y = height;
    }
}