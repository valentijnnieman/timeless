#pragma once
#include <string>
#include "glm/vec2.hpp"

class Event
{
public:
    std::string eventType;
    virtual void getData()
    {
        return;
    }
    Event()
    {
        eventType = "None";
    }
    Event(std::string et)
        : eventType(et)
    {
    }
};

class MouseEvent : public Event
{
public:
    glm::vec2 screen_position;
    bool picked_up = false;
    MouseEvent(std::string et, glm::vec2 sp)
        : Event(et), screen_position(sp)
    {
    }
};

class KeyboardEvent : public Event
{
public:
    KeyboardEvent(std::string et)
        : Event(et)
    {
    }
};


class PositionEvent : public Event
{
public:
    glm::vec2 position;
    PositionEvent(std::string et, glm::vec2 position)
        : Event(et), position(position)
    {
    }
};

class Component;

class CollisionEvent : public Event
{
public:
    Component &a;
    Component &b;

    CollisionEvent(std::string et, Component &a, Component &b)
        : a(a), b(b), Event(et)
    {
    }
};

class HighlightEvent : public Event
{
public:
    std::string label;

    HighlightEvent(std::string et, std::string label)
        : label(label), Event(et)
    {
    }
};

class MoveEvent : public Event
{
public:
    Component &a;
    int x, y;

    MoveEvent(std::string et, Component &a, int x, int y)
        : a(a), x(x), y(y), Event(et)
    {
    }
};