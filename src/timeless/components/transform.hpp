#pragma once
#include "glm/glm.hpp"
#include <glm/vec2.hpp>
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <queue>
#include <iostream>
#include "timeless/settings.hpp"

class Transform
{
private:
    std::queue<glm::vec3> movement_frames;

public:
    glm::vec3 camera_position = glm::vec3(0.0f);
    glm::mat4 model, view, projection;

    glm::mat4 light_view;
    glm::mat4 light_projection;
    glm::mat4 light_space;

    glm::vec3 position;
    glm::vec3 start_position;
    glm::vec3 offset;
    glm::vec3 scale;
    glm::quat rot;
    float rotation;
    float width, height;
    float grid_x = 0;
    float grid_y = 0;
    bool flip = false;
    bool center = true;

    Transform(glm::vec3 p, float r, float w, float h, glm::vec3 o = glm::vec3(0.0f), bool center = true)
        : position(p), start_position(p),
          rotation(r), width(w), height(h), offset(o), center(center), scale(glm::vec3(w, h, 1.0f))
    {
        rot = glm::quat(glm::vec3(0, 0, 0));
        model = glm::mat4(1.0f);
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

    void rotate(glm::vec3 eulers)
    {
        rot = glm::quat(eulers);
    }
    void set_position(glm::vec3 p)
    {
        position = p;
    }
    void set_offset(glm::vec3 o)
    {
        offset = o;
    }
    void set_scale(glm::vec3 s)
    {
        scale = s;
        width = s.x;
        height = s.y;
    }
    void set_movement_frames(glm::vec3 destination)
    {
        //while (!movement_frames.empty())
        //    movement_frames.pop();

        //glm::vec3 start = position;

        //for (double i = 0.0; i <= 1.0; i += 1.0 / 60.0)
        //{
        //    double x_l = std::lerp(start.x, destination.x, i);
        //    double y_l = std::lerp(start.y, destination.y, i);
        //    movement_frames.push(glm::vec3(x_l, y_l, destination.z));
        //}
    }

    void update_camera(glm::vec3 p)
    {
        camera_position = p;
    }

    glm::vec3 get_position_minus_offset()
    {
        return glm::vec3((position.x - offset.x), (position.y - offset.y), position.z);
    }

    /* returns position minus camera - i.e. position as if looking from
    camera's perspective */
    glm::vec3 get_position_from_camera()
    {
        return glm::vec3((position.x - offset.x) - camera_position.x, (position.y - offset.y) - camera_position.y, position.z - camera_position.z);
    }

    glm::vec3 get_centered_position_from_camera()
    {
        return glm::vec3((position.x - offset.x) - camera_position.x, (position.y - offset.y) - camera_position.y, position.z - camera_position.z) + glm::vec3(0.5 * width, 0.5 * height, 0.0);
    }

    void update(int x = TESettings::SCREEN_X, int y = TESettings::SCREEN_Y, float zoom = 1.0f, glm::vec3 offset = glm::vec3(0.0f))
    {
        projection = glm::ortho(0.0f, static_cast<float>(x) * zoom, static_cast<float>(y) * zoom, 0.0f, -1000.0f, 1000.0f);

        // view = glm::mat4(1.0f);
        // glm::mat4 viewTransform = glm::mat4(1.0f);
        // viewTransform = glm::translate(viewTransform, camera_position);
        //
        // view = viewTransform * view;
        view = glm::lookAt(camera_position, camera_position + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

        model = glm::mat4(1.0f);
        glm::mat4 modelTransform = glm::mat4(1.0f);
        modelTransform = glm::translate(modelTransform, get_position_minus_offset());

        if (center)
        {
            model = glm::translate(model, glm::vec3(0.5 * width, 0.5 * height, 0.0));
        }
        else
        {
            model = glm::translate(model, glm::vec3(0.5 * width, 0.0, 0.0));
        }
        model = glm::translate(model, offset);
        glm::mat4 rotation_matrix = glm::toMat4(rot);
        model = modelTransform * rotation_matrix * model;

        if (flip)
        {
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        model = glm::scale(model, scale);
    }
};
