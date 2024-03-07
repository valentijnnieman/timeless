#pragma once
#include "system.hpp"

class AnimationSystem : public System
{
public:
	AnimationSystem() {};

    void update(ComponentManager &cm)
    {
        for (auto entity : registered_entities)
        {
            auto animation = cm.animations.at(entity);
            if (!animation->positions.empty())
            {
                auto transform = cm.transforms.at(entity);
				glm::vec3 dir = animation->positions.front();
                transform->set_position(dir);
                animation->positions.pop();
            }
            if (!animation->scales.empty())
            {
                auto transform = cm.transforms.at(entity);
				glm::vec3 scale = animation->scales.front();
                transform->set_scale(scale);
                animation->scales.pop();
            }

            if (!animation->rotations.empty())
            {
                auto transform = cm.transforms.at(entity);
				glm::vec3 euler_angles = animation->rotations.front();
                transform->rotate(euler_angles);
                animation->rotations.pop();
                if (animation->loop)
                {
					animation->rotations.push(euler_angles);
                }
            }

        }
    }
};
