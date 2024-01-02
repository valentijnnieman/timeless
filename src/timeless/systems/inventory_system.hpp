#pragma once
#include <vector>
#include <string>
#include "../entity.hpp"

class InventorySystem
{
public:
    std::vector<Entity> registered_entities;
    int selected = 0;

    void register_entity(Entity entity)
    {
        registered_entities.push_back(entity);
        selected = registered_entities.size() - 1;
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

    Entity get_current()
    {
        try
        {
            if (registered_entities.size() > selected)
            {
                return registered_entities[selected];
            }
            else
            {
                throw std::out_of_range("could not find inventory item");
            }
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Exception at " << e.what() << std::endl;
            return 0;
        }
    }
};