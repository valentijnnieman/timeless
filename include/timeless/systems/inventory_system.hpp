#pragma once
#include <vector>
#include <string>
#include "timeless/systems/system.hpp"

class InventorySystem : public System
{
public:
    int selected = 0;

    Entity get_current();
};