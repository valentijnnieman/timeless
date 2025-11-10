#pragma once
#include "timeless/components/component.hpp"

class MovementController : public Component
{
private:
    bool can_control = true;
};
