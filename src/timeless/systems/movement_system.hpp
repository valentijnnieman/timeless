#pragma once
#include "glm/glm.hpp"
#include <array>
#include "system.hpp"

class MovementSystem : public System
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

    glm::vec2 x_bounds;
    glm::vec2 y_bounds;
    float walk_speed;
    float camera_speed;
    std::array<bool, 6> keysPressed = {false, false, false, false, false};

public:
    Entity camera;

    MovementSystem()
        : x_bounds(glm::vec2(-TESettings::SCREEN_X, TESettings::SCREEN_X)), y_bounds(glm::vec2(-TESettings::SCREEN_Y, TESettings::SCREEN_Y)), walk_speed(1.0f), camera_speed(10.0f)
    {
    }

    void register_camera(Entity c)
    {
        camera = c;
    }

    glm::vec4 calculate_collider(glm::vec3 position, float width, float height)
    {
        float x = position.x;
        float y = position.y;

        return glm::vec4(x, x + width, y, y + height);
    }

    void move_right(ComponentManager &cm)
    {
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.sprites.at(entity);
            auto transform = cm.transforms.at(entity);
            auto collider = cm.colliders.at(entity);

            if (transform->position.x + walk_speed < x_bounds[1])
            {
                glm::vec4 colPos = calculate_collider(glm::vec3(transform->position.x + walk_speed, transform->position.y, 0.0), transform->width, transform->height);
                collider->setBounds(colPos);
                // int gridLayer = getParentScene()->findClosestTile(collider)->layer;
                int grid_layer = 0;
                if (grid_layer != 1)
                {
                    transform->position.x += walk_speed;
                    sprite->animating = true;
                    sprite->flip = true;
                    // playWalking(gridLayer);
                    // update("PlayerPosition", position);
                }
            }
        }

        if (cm.cameras.count(camera) != 0)
        {
            cm.get_camera(camera)->position.x += camera_speed;
        }
    }

    void move_left(ComponentManager &cm)
    {
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.sprites.at(entity);
            auto transform = cm.transforms.at(entity);
            auto collider = cm.colliders.at(entity);

            if (transform->position.x - walk_speed > x_bounds[0])
            {
                glm::vec4 colPos = calculate_collider(glm::vec3(transform->position.x - walk_speed, transform->position.y, 0.0), transform->width, transform->height);
                collider->setBounds(colPos);
                // int gridLayer = getParentScene()->findClosestTile(collider)->layer;
                int grid_layer = 0;
                if (grid_layer != 1)
                {
                    transform->position.x -= walk_speed;
                    sprite->animating = true;
                    sprite->flip = true;
                    // playWalking(gridLayer);
                    // update("PlayerPosition", position);
                }
            }
        }

        if (cm.cameras.count(camera) != 0)
        {
            cm.get_camera(camera)->position.x -= camera_speed;
        }
    }
    void move_down(ComponentManager &cm)
    {
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.sprites.at(entity);
            auto transform = cm.transforms.at(entity);
            auto collider = cm.colliders.at(entity);

            if (transform->position.y + walk_speed < y_bounds[1])
            {
                glm::vec4 colPos = calculate_collider(glm::vec3(transform->position.x, transform->position.y + walk_speed, 0.0), transform->width, transform->height);
                collider->setBounds(colPos);
                // int gridLayer = getParentScene()->findClosestTile(collider)->layer;
                int grid_layer = 0;
                if (grid_layer != 1)
                {
                    transform->position.y += walk_speed;
                    sprite->animating = true;
                    // playWalking(gridLayer);
                    // update("PlayerPosition", position);
                }
            }
        }

        if (cm.cameras.count(camera) != 0)
        {
            cm.get_camera(camera)->position.y += camera_speed;
        }
    }

    void move_up(ComponentManager &cm)
    {
        for (auto &entity : registered_entities)
        {
            auto sprite = cm.sprites.at(entity);
            auto transform = cm.transforms.at(entity);
            auto collider = cm.colliders.at(entity);

            if (transform->position.y - walk_speed > y_bounds[0])
            {
                glm::vec4 colPos = calculate_collider(glm::vec3(transform->position.x, transform->position.y + walk_speed, 0.0), transform->width, transform->height);
                collider->setBounds(colPos);
                // int gridLayer = getParentScene()->findClosestTile(collider)->layer;
                int grid_layer = 0;
                if (grid_layer != 1)
                {
                    transform->position.y += walk_speed;
                    sprite->animating = true;
                    // playWalking(gridLayer);
                    // update("PlayerPosition", position);
                }
            }
        }

        if (cm.cameras.count(camera) != 0)
        {
            cm.get_camera(camera)->position.y -= camera_speed;
        }
    }

    void update(ComponentManager &cm, GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            move_right(cm);
            keysPressed[d] = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            move_left(cm);
            keysPressed[a] = true;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            move_up(cm);
            keysPressed[w] = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            move_down(cm);
            keysPressed[s] = true;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            keysPressed[escape] = true;
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE && keysPressed[escape])
        {
            keysPressed[escape] = false;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && keysPressed[a])
        {
            // update("StopWalking");
            keysPressed[a] = false;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && keysPressed[w])
        {

            // update("StopWalking");
            keysPressed[w] = false;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && keysPressed[s])
        {
            // update("StopWalking");
            keysPressed[s] = false;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && keysPressed[d])
        {
            // update("StopWalking");
            keysPressed[d] = false;
        }
    }
};