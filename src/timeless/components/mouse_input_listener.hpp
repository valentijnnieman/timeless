#pragma once
#include <functional>
#include "../entity.hpp"
#include "../event.hpp"

class MouseInputListener
{
private:
    std::function<void(MouseEvent *event, Entity entity, int data)> on_click;

public:
    MouseInputListener(std::function<void(MouseEvent *event, Entity entity, int data)> onClick)
        : on_click(onClick)
    {
    }
    void on_click_handler(MouseEvent *event, Entity entity, int data)
    {
        on_click(event, entity, data);
    }
};