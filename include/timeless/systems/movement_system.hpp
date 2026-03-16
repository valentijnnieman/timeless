#pragma once
#include <glm/glm.hpp>
#include <array>
#include "timeless/components/transform.hpp"
#include "timeless/systems/system.hpp"

class MovementSystem : public System
{
private:
    enum Keys
    {
        w,
        a,
        s,
        d,
        escape,
        up,
        down,
        left,
        right
    };

    glm::vec2 x_bounds;
    glm::vec2 y_bounds;
    float walk_speed;
    float camera_speed;
    std::array<bool, 10> keysPressed = {false, false, false, false, false, false, false, false, false, false};

public:
    Entity camera;

    MovementSystem();
    void register_camera(Entity c);
    glm::vec4 calculate_collider(glm::vec3 position, float width, float height);
    void move_right(ComponentManager &cm);
    void move_left(ComponentManager &cm);
    void move_down(ComponentManager &cm);
    void move_up(ComponentManager &cm);
    void update(ComponentManager &cm, GLFWwindow *window);
};
