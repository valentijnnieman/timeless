#pragma once
#include <functional>
#include "timeless/entity.hpp"
#include "timeless/event.hpp"
#include "timeless/components/component.hpp"

class KeyboardInputListener : public Component
{
private:
    std::function<void(KeyboardEvent *event, Entity entity, int data)> on_input;

public:
    KeyboardInputListener(std::function<void(KeyboardEvent *event, Entity entity, int data)> on_input_func)
        : on_input(on_input_func)
    {
    }
    void on_input_handler(KeyboardEvent* event, Entity entity, int data)
    {
        on_input(event, entity, data);
    }
};
