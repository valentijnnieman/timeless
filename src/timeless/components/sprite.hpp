#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <glm/vec2.hpp>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../timer.hpp"

#include "stb_image.h"

class Sprite
{
private:
    float width, height;
    Timer animationSpeed = Timer(0.075f);
    bool isStatic = true;
    int frames;

public:
    bool animating;
    bool flip = false;
    bool hidden = false;

    glm::vec4 color;
    float index;
    glm::vec2 spriteSheetSize;
    glm::vec2 spriteSize;

    Sprite(float index, glm::vec4 color, float width, float height, glm::vec2 spriteSheetSize, glm::vec2 spriteSize, bool isStatic = true, bool animating = false, int frames = 6)
        : color(color), index(index), width(width), height(height), isStatic(isStatic), animating(animating), frames(frames), spriteSheetSize(spriteSheetSize), spriteSize(spriteSize)
    {
    }
    void setColor(glm::vec4 newColor)
    {
        color = newColor;
    }
    void setIndex(float newIndex)
    {
        index = newIndex;
    }
    void update()
    {
        if (animationSpeed.pollTime() && animating)
        {
            index += 1;
            if (index >= frames)
            {
                index = 0;
            }
        }
        else if (!animating && !isStatic)
        {
            index = 0;
        }
    }
    void render()
    {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};