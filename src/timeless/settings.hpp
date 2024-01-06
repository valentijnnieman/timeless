
namespace TESettings
{
    static glm::vec4 SCREEN_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
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