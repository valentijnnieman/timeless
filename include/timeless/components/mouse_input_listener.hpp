#pragma once
#include <functional>
#include "timeless/entity.hpp"
#include "timeless/event.hpp"
#include "timeless/components/component.hpp"

template <typename T = MouseEvent>
class MouseInputListener : public Component
{
private:
    std::function<void(T *event, Entity entity, int data)> on_click;

public:
    // Which listeners hear an event first: higher layers before lower ones
    // (see MouseInputSystem). Within a layer they keep the system's order,
    // registration or sort(). E.g. put the world below the UI, so a panel
    // can claim (picked_up) a click before whatever is behind it hears it.
    int layer = 0;

    MouseInputListener(std::function<void(T *event, Entity entity, int data)> onClick, int layer = 0)
        : on_click(onClick), layer(layer)
    {
    }
    void on_click_handler(T *event, Entity entity, int data)
    {
        on_click(event, entity, data);
    }
};
