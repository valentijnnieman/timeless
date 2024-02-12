#pragma once
#include "glm/glm.hpp"
#include <array>

class KeyboardInputSystem 
{
private:
    enum Keys
    {
        w,
        a,
        s,
        d,
        escape
    };

    std::array<bool, 5> keysPressed = {false, false, false, false, false};

public:
    std::vector<Entity> registered_entities;

    void register_entity(Entity entity)
    {
        registered_entities.push_back(entity);
    }
    void remove_entity(Entity entity)
    {
        if (!registered_entities.empty())
        {
            auto found = std::find_if(registered_entities.begin(), registered_entities.end(), [&](auto &e)
                                      { return e == entity; });
            if (found != registered_entities.end())
            {
                registered_entities.erase(found);
            }
        }
    }    
    void notify_listener(ComponentManager &cm, KeyboardEvent *event, Entity entity)
    {
        auto listener = cm.get_keyboard_input_listener(entity);
        listener->on_input_handler(event, entity, 0);
    }


    glm::vec4 calculate_collider(glm::vec3 position, float width, float height)
    {
        float x = position.x;
        float y = position.y;

        return glm::vec4(x, x + width, y, y + height);
    }

    void update(ComponentManager &cm, GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            keysPressed[escape] = true;

            for (auto& entity : registered_entities)
            {
                notify_listener(cm, new KeyboardEvent("PressEscape"), entity);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE && keysPressed[escape])
        {
            keysPressed[escape] = false;

            for (auto& entity : registered_entities)
            {
                notify_listener(cm, new KeyboardEvent("ReleaseEscape"), entity);
            }
        }
    }
};