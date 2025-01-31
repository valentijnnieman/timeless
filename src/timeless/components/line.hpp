#pragma once
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include "component.hpp"

class Line : public Component
{
private:
    unsigned int VAO, VBO;

    std::vector<float> vertices;

public:
    glm::vec3 start;
    glm::vec3 end;
    Line(glm::vec3 s, glm::vec3 e)
        : start(s), end(e)
    {
        vertices = {
            start.x,
            start.y,
            start.z,
            end.x,
            end.y,
            end.z,

        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    ~Line()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    void render()
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 2);
    }
};
