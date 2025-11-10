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
    MouseInputListener(std::function<void(T *event, Entity entity, int data)> onClick)
        : on_click(onClick)
    {
    }
    void on_click_handler(T *event, Entity entity, int data)
    {
        on_click(event, entity, data);
    }
};
