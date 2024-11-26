#pragma once
#include "component.hpp"

class Collider : public Component
{
public:
    int layer;
    float x1, x2, y1, y2;
    bool colliding;
    Collider()
    {
    }
    Collider(float x1, float x2, float y1, float y2, int layer)
        : x1(x1), x2(x2), y1(y1), y2(y2), layer(layer), colliding(false)
    {
    }
    void setBounds(float nx1, float nx2, float ny1, float ny2)
    {
        x1 = nx1;
        x2 = nx2;
        y1 = ny1;
        y2 = ny2;
    }
    void setBounds(glm::vec4 bounds)
    {
        x1 = bounds.x;
        x2 = bounds.y;
        y1 = bounds.z;
        y2 = bounds.w;
    }
};
