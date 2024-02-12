#pragma once

class AnimationSystem
{
public:
	AnimationSystem() {};

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

    void update(ComponentManager &cm)
    {
        for (auto entity : registered_entities)
        {
            auto animation = cm.animations.at(entity);
            if (!animation->positions.empty())
            {
                auto transform = cm.transforms.at(entity);
				glm::vec3 dir = animation->positions.front();
                transform->set(dir);
                animation->positions.pop();
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
