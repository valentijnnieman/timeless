#pragma once
#include "../event.hpp"
#include "../timeless.hpp"

class MouseInputSystem
{
public:
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

    void notify_listener(ComponentManager &cm, MouseEvent *event, Entity entity)
    {
        auto listener = cm.get_mouse_input_listener(entity);
        listener->on_click_handler(event, entity, 0);
    }

    void mouse_click_handler(ComponentManager &cm, MouseEvent *event)
    {
        bool found_entity = false;
        double normalizedX = (double)event->mousePosition.x / TESettings::SCREEN_X;
        double normalizedY = (double)event->mousePosition.y / TESettings::SCREEN_Y;

        glm::vec2 m_pos(normalizedX * TESettings::VIEWPORT_X, normalizedY * TESettings::VIEWPORT_Y);

        // for (const auto &[entity, transform] : transforms)
        for (const auto &entity : registered_entities)
        {
            auto transform = cm.get_transform(entity);
            if (transform != nullptr)
            {
                glm::vec3 t_pos = transform->get_position_from_camera();
                if (m_pos.x > (t_pos.x - transform->width) && m_pos.x < (t_pos.x + transform->width) &&
                    m_pos.y > (t_pos.y - transform->height) && m_pos.y < (t_pos.y + transform->height))
                {

                    notify_listener(cm, event, entity);
                    found_entity = true;
                    break;
                }
            }
        }

        // if no entity was found in transforms, we treat it as a "global" click
        if (!found_entity)
        {
            std::string et = event->eventType;
            for (const auto &entity : registered_entities)
            {
                event->eventType = "Global" + et;
                notify_listener(cm, event, entity);
            }
        }
    }
    void mouse_release_handler(ComponentManager &cm, MouseEvent *event)
    {
        for (const auto &entity : registered_entities)
        {
            notify_listener(cm, event, entity);
        }
    }
    void mouse_move_handler(ComponentManager &cm, MouseEvent *event)
    {
        double normalizedX = (double)event->mousePosition.x / TESettings::SCREEN_X;
        double normalizedY = (double)event->mousePosition.y / TESettings::SCREEN_Y;
        glm::vec2 m_pos(normalizedX * TESettings::VIEWPORT_X, normalizedY * TESettings::VIEWPORT_Y);

        event->mousePosition = m_pos;

        for (const auto &entity : registered_entities)
        {
            notify_listener(cm, event, entity);
        }
    }
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        // ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X * yoffset) * 0.5f;
        // ViewportSettings::SCR_VIEWPORT_Y += (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
    }
};