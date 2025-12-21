#pragma once

#include <iostream>
#include "timeless/components/component.hpp"

class Quad : public Component
{
private:
    unsigned int VAO, EBO, VBO;

    const float vertices[32] = {
        // positions          // colors           // texture coords
        1.0f, 1.0f, 0.0f, 1.0, 1.0, 1.0, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0, 1.0, 1.0, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 1.0, 1.0, 1.0, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 1.0, 1.0, 1.0, 0.0f, 1.0f   // top left
    };
    const unsigned int indices[6] = {
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };

public:
    Quad();
    ~Quad();
    
    void attributes();
    void render();
};
