#pragma once
#include "component.hpp"
#include "glm/glm.hpp"
#include "timeless/entity.hpp"
#include <cmath>
#include <queue>

class AnimationFrame
{
public:
};

class Animation : public Component
{
private:
    Entity entity;
public:
	std::queue<glm::vec3> positions;
	std::queue<glm::vec3> scales;
	std::queue<glm::vec3> rotations;

	std::queue<float> opacities;

    bool loop = false;
    bool triggered = false;

    void set_position_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0)
    {
        while (!positions.empty())
            positions.pop();

        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from.x, to.x, i);
            double y_l = std::lerp(from.y, to.y, i);
            double z_l = std::lerp(from.z, to.z, i);
            positions.push(glm::vec3(x_l, y_l, z_l));
        }
    }

    void set_opacity_frames(float from, float to, float speed = 1.0)
    {
        while (!opacities.empty())
            opacities.pop();

        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from, to, i);
            opacities.push(x_l);
        }
    }

    void append_opacity_frames(float from, float to, float speed = 1.0)
    {
        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from, to, i);
            opacities.push(x_l);
        }
    }

    void set_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0)
    {
        while (!rotations.empty())
            rotations.pop();

        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from.x, to.x, i);
            double y_l = std::lerp(from.y, to.y, i);
            double z_l = std::lerp(from.z, to.z, i);
            rotations.push(glm::vec3(x_l, y_l, z_l));
        }
    }

    void append_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0)
    {
        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from.x, to.x, i);
            double y_l = std::lerp(from.y, to.y, i);
            double z_l = std::lerp(from.z, to.z, i);
            rotations.push(glm::vec3(x_l, y_l, z_l));
        }
    }

    void set_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0)
    {
        while (!scales.empty())
            scales.pop();

        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from.x, to.x, i);
            double y_l = std::lerp(from.y, to.y, i);
            double z_l = std::lerp(from.z, to.z, i);
            scales.push(glm::vec3(x_l, y_l, z_l));
        }
    }
    void append_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0)
    {
        for (double i = 0.0; i <= 1.0; i += speed / 60.0)
        {
            double x_l = std::lerp(from.x, to.x, i);
            double y_l = std::lerp(from.y, to.y, i);
            double z_l = std::lerp(from.z, to.z, i);
            scales.push(glm::vec3(x_l, y_l, z_l));
        }
    }


};
