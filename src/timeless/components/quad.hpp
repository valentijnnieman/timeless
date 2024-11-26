#pragma once
#include "glad/glad.h"
#include "component.hpp"

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
    Quad()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    ~Quad()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
    void render()
    {
        glBindVertexArray(VAO);
    }
};
