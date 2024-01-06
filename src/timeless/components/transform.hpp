#pragma once
#include "glm/glm.hpp"
#include <glm/vec2.hpp>
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <queue>
#include <iostream>
#include "timeless/settings.hpp"

class Transform
{
private:
    glm::vec3 camera_position = glm::vec3(0.0f);
    std::queue<glm::vec3> movement_frames;

public:
    glm::mat4 model, view, projection;
    glm::vec3 position;
    glm::vec3 offset;
    float rotation;
    float width, height;
    bool flip = false;

    Transform(glm::vec3 p, float r, float w, float h, glm::vec3 o = glm::vec3(0.0f))
        : position(p),
          rotation(r), width(w), height(h), offset(o)
    {
        // projection = glm::ortho(0.0f, static_cast<float>(SCR_VIEWPORT_X), static_cast<float>(SCR_VIEWPORT_Y), 0.0f, -1.0f, 1.0f);

        model = glm::mat4(1.0f);

        glm::mat4 transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::translate(transformMatrix, position);
        model = transformMatrix * model;

        // model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        // model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.0f));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));

        view = glm::mat4(1.0f);
    }
    void translate(glm::vec3 p)
    {
        position += p;
    }
    void rotate(float r)
    {
        rotation = r;
    }
    void set(glm::vec3 p)
    {
        position = p;
    }
    void set_movement_frames(glm::vec3 destination)
    {
        while (!movement_frames.empty())
            movement_frames.pop();

        glm::vec3 start = position;

        for (double i = 0.0; i <= 1.0; i += 1.0 / 60.0)
        {
            double x_l = std::lerp(start.x, destination.x, i);
            double y_l = std::lerp(start.y, destination.y, i);
            movement_frames.push(glm::vec3(x_l, y_l, destination.z));
        }
    }

    void update_camera(glm::vec3 p)
    {
        camera_position = p;
    }

    /* returns position minus camera - i.e. position as if looking from
    camera's perspective */
    glm::vec3 get_position_from_camera()
    {
        return glm::vec3((position.x - offset.x) - camera_position.x, (position.y - offset.y) - camera_position.y, position.z - camera_position.z);
    }

    void update(int x = TESettings::SCREEN_X, int y = TESettings::SCREEN_Y)
    {
        view = glm::mat4(1.0f);
        projection = glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f, -100.0f, 100.0f);

        model = glm::mat4(1.0f);
        glm::mat4 transformMatrix = glm::mat4(1.0f);
        if (!movement_frames.empty())
        {
            glm::vec3 dir = movement_frames.front();
            set(dir);
            movement_frames.pop();
        }

        transformMatrix = glm::translate(transformMatrix, get_position_from_camera());
        // transformMatrix = glm::translate(transformMatrix, position);

        model = transformMatrix * model;
        model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.0f));

        model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        if (flip)
        {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        model = glm::scale(model, glm::vec3(width, height, 1.0f));
    }
};