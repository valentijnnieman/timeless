#pragma once
#include "../timer.hpp"

class Text
{
private:
    std::unique_ptr<Timer> type_timer;

public:
    std::string text;
    std::string printed;
    glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool center;

    float type_speed = 0.0f;
    bool completed;
    int print_length = 0;

    Text(std::string text, bool center = true, glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f), float type_speed = 0.0f)
        : text(text), center(center), color(glm::vec4(color.x, color.y, color.z, 1.0f)), type_speed(type_speed)
    {
        if (type_speed > 0.0)
        {
            type_timer = std::unique_ptr<Timer>(new Timer(type_speed));
            printed = "";
        }
        else
        {
            printed = text;
        }
    }

    void render(Font &font, int x, int y, float height)
    {
        // iterate through all characters
        std::string::const_iterator c;

        if (type_timer != nullptr)
        {
            if (print_length < text.length() && type_timer->pollTime())
            {
                print_length += 1;

                // // play sound if set
                // if (typingDescription != NULL)
                // {
                //     FMOD::Studio::EventInstance *eventInstance = NULL;
                //     typingDescription->createInstance(&eventInstance);

                //     eventInstance->start();
                //     eventInstance->release();
                // }
            }
            else if (!completed && print_length >= text.length())
            {
                completed = true;
            }
            printed = text.substr(0, print_length);
        }

        for (c = printed.begin(); c != printed.end(); c++)
        {
            Glyph glyph = font.glyphs.at(*c);

            float w = glyph.size.x;
            float h = glyph.size.y;

            float xpos = x + glyph.bearing.x;
            float ypos = y - (glyph.bearing.y) + height;
            // update VBO for each character
            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 1.0f},
                {xpos, ypos, 0.0f, 0.0f},
                {xpos + w, ypos, 1.0f, 0.0f},

                {xpos, ypos + h, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 0.0f},
                {xpos + w, ypos + h, 1.0f, 1.0f}};
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, glyph.textureId);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, font.VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

            // glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (glyph.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
    }
};