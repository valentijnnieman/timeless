#pragma once
#include "../event.hpp"

namespace MouseInputSystem
{
    // std::shared_ptr<ComponentManager> cm;
    std::unordered_map<Entity, std::shared_ptr<Transform>> transforms;
    std::unordered_map<Entity, std::shared_ptr<Transform>> ui_transforms;
    std::unordered_map<Entity, std::shared_ptr<MouseInputListener>> listeners;

    void register_listener(Entity entity, std::shared_ptr<MouseInputListener> listener)
    {
        listeners.insert({entity, listener});
    }
    void register_transform(Entity entity, std::shared_ptr<Transform> transform)
    {
        if (transform != nullptr)
        {
            transforms.insert({entity, transform});
        }
    }
    void register_ui_transform(Entity entity, std::shared_ptr<Transform> transform)
    {
        if (transform != nullptr)
        {
            ui_transforms.insert({entity, transform});
        }
    }

    void remove_entity(Entity entity)
    {
        listeners.erase(entity);
        transforms.erase(entity);
        ui_transforms.erase(entity);
    }

    void notify_listener(MouseEvent *event, Entity entity)
    {
        listeners.at(entity)->on_click_handler(event, entity, 0);
    }

    void mouse_click_handler(MouseEvent *event)
    {
        bool found_entity = false;
        double normalizedX = (double)event->mousePosition.x / TESettings::SCREEN_X;
        double normalizedY = (double)event->mousePosition.y / TESettings::SCREEN_Y;

        glm::vec2 m_pos(normalizedX * TESettings::VIEWPORT_X, normalizedY * TESettings::VIEWPORT_Y);

        for (const auto &[entity, transform] : transforms)
        {
            if (transform != nullptr)
            {
                glm::vec3 t_pos = transform->get_position_from_camera();
                if (m_pos.x > (t_pos.x - transform->width) && m_pos.x < (t_pos.x + transform->width) &&
                    m_pos.y > (t_pos.y - transform->height) && m_pos.y < (t_pos.y + transform->height))
                {

                    notify_listener(event, entity);
                    found_entity = true;
                    break;
                }
            }
        }
        if (!found_entity)
        {
            for (const auto &[entity, transform] : ui_transforms)
            {
                if (transform != nullptr)
                {
                    glm::vec3 t_pos = transform->get_position_from_camera();
                    if (m_pos.x > (t_pos.x - transform->width) && m_pos.x < (t_pos.x + transform->width) &&
                        m_pos.y > (t_pos.y - transform->height) && m_pos.y < (t_pos.y + transform->height))
                    {
                        notify_listener(event, entity);
                        found_entity = true;
                        break;
                    }
                }
            }
        }

        // if no entity was found in both normal and ui transforms, we treat it as a "global" click
        if (!found_entity)
        {
            for (const auto &[entity, listener] : listeners)
            {
                event->eventType = "Global" + event->eventType;
                notify_listener(event, entity);
            }
        }
    }
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        // ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X * yoffset) * 0.5f;
        // ViewportSettings::SCR_VIEWPORT_Y += (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
    }
};