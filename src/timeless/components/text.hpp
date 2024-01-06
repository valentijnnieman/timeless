#pragma once

class Text
{
public:
    std::string text;
    std::string printed;
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
    bool center;

    float type_speed = 0.0f;
    bool completed;
    int print_length = 0;

    Text(std::string t, bool c = true, glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f))
        : text(t), center(c), color(color)
    {
        if (type_speed > 0.0)
        {
            // if (print_length < text.length() && timer.pollTime())
            // {
            //     print_length += 1;

            //     // // play sound if set
            //     // if (typingDescription != NULL)
            //     // {
            //     //     FMOD::Studio::EventInstance *eventInstance = NULL;
            //     //     typingDescription->createInstance(&eventInstance);

            //     //     eventInstance->start();
            //     //     eventInstance->release();
            //     // }
            // }
            // else if (!completed && printLength >= text.length())
            // {
            //     completed = true;
            // }
            // printed = text.substr(0, printLength);
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