#pragma once
#include "timeless/timer.hpp"
#include "timeless/components/component.hpp"

class Text : public Component
{
private:
    std::unique_ptr<Timer> type_timer;
    std::function<void()> typing_callback;
    std::queue<float> opacities;

public:
    std::string text;
    std::string printed;
    std::vector<glm::vec4> color_vector;

    glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool center;
    bool hidden = false;

    float type_speed = 0.0f;
    bool completed;
    int print_length = 0;

    Text(std::string text, bool center = true, glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f), float type_speed = 0.0f, std::function<void()> typing_callback = []() {})
        : text(text), center(center), color(glm::vec4(color.x, color.y, color.z, 1.0f)), type_speed(type_speed), typing_callback(typing_callback)
    {
        set_default_color_vector();
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

    void set_default_color_vector()
    {
        for (int i = 0; i < text.size(); i++)
	    {
            color_vector.push_back(color);
        }
    }

    void set_opacity_frames(float from, float to, float speed = 1.0) {
      while (!opacities.empty())
        opacities.pop();

      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from, to, i);
        opacities.push(x_l);
      }
    }

    void append_opacity_frames(float from, float to, float speed = 1.0) {
      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from, to, i);
        opacities.push(x_l);
      }
    }

    void render(Font &font, int x, int y, float height, std::shared_ptr<Shader> shader)
    {

        if (!opacities.empty()) {
          float o = opacities.front();
          color.a = o;
          for (auto &c : color_vector) {
            c.a = o;
          }
          opacities.pop();
        }
        // iterate through all characters
        std::string::const_iterator c;

        if (type_timer != nullptr)
        {
            if (print_length < text.length() && type_timer->pollTime())
            {
                print_length += 1;

                typing_callback();
            }
            else if (!completed && print_length >= text.length())
            {
                completed = true;
            }
            printed = text.substr(0, print_length);
        }

        int i = 0;
        for (c = printed.begin(); c != printed.end(); c++)
        {
            if (printed.size() <= color_vector.size())
            {
                glUniform4f(glGetUniformLocation(shader->ID, "textColor"), this->color_vector[i].r, this->color_vector[i].g, this->color_vector[i].b, this->color_vector[i].a);
            }
            else
            {
                set_default_color_vector();
            }
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

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (glyph.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            i++;
        }
    }
};
