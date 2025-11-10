#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <glm/vec2.hpp>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "timeless/timer.hpp"
#include "timeless/components/component.hpp"

#include "stb_image.h"

class Sprite : public Component
{
private:
    Timer animationSpeed = Timer(0.075f);
    bool isStatic = true;
    int frames;
    int og_index;

    std::queue<float> opacities;

public:
    bool animating;
    bool flip = false;
    bool hidden = false;

    glm::vec4 color;
    glm::vec4 og_color;
    int index;
    glm::vec2 spriteSheetSize;
    glm::vec2 spriteSize;

    Sprite(int index, glm::vec4 color, glm::vec2 spriteSheetSize, glm::vec2 spriteSize, bool isStatic = true, bool animating = false, int frames = 6)
        : color(color), og_color(color), index(index), og_index(index), isStatic(isStatic), animating(animating), frames(frames), spriteSheetSize(spriteSheetSize), spriteSize(spriteSize)
    {
    }
    void setColor(glm::vec4 newColor)
    {
        color = newColor;
    }
    void set_index(int newIndex)
    {
        index = newIndex;
        og_index = newIndex;
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
    void update()
    {
        if (animationSpeed.pollTime() && animating)
        {
            index += 1;
            if (index >= (og_index + frames))
            {
                index = og_index;
            }
        }
        else if (!animating && !isStatic)
        {
            index = og_index;
        }

        if (!opacities.empty()) {
          float o = opacities.front();
          color.a = o;
          opacities.pop();

        }
    }
    void render()
    {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};
