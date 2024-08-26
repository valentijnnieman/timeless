#pragma once
#include <vector>
#include <string>
#include "system.hpp"

class InventorySystem : public System
{
public:
    int selected = 0;

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