#pragma once
#include <glm/glm.hpp>
#include <array>
#include "timeless/systems/system.hpp"

class KeyboardInputSystem : public System
{
private:
    enum Keys
    {
        w,
        a,
        s,
        d,
        escape,
        space,
        enter,
        f,
        key_count
    };

    std::array<bool, key_count> keysPressed = {};

public:

    void notify_listener(ComponentManager &cm, KeyboardEvent *event, Entity entity);
    glm::vec4 calculate_collider(glm::vec3 position, float width, float height);
    void update(ComponentManager &cm, WindowManager &wm);
};
