#pragma once
#include "timeless/components/component.hpp"
#include <glm/glm.hpp>
#include <vector>

class Line : public Component
{
private:
    unsigned int VAO, VBO;

    std::vector<float> vertices;

public:
    glm::vec3 start;
    glm::vec3 end;
    Line(glm::vec3 s, glm::vec3 e);
    ~Line();
    void render();
};
