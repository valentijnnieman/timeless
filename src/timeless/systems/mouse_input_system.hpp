#pragma once
#include "../event.hpp"

namespace MouseInputSystem
{
    // std::shared_ptr<ComponentManager> cm;
    std::unordered_map<Entity, std::shared_ptr<Transform>> transforms;
    std::unordered_map<Entity, std::shared_ptr<Transform>> ui_transforms;
    std::unordered_map<Entity, std::shared_ptr<MouseInputListener>> listeners;

    int selected_index = 0;
    int selected_layer = 1;

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

    void notify_listener(MouseEvent *event, Entity entity, int data)
    {
        listeners.at(entity)->on_click_handler(event, entity, data);
    }

    void mouse_click_handler(MouseEvent *event)
    {
        bool found_entity = false;
        for (const auto &[entity, transform] : transforms)
        {
            if (transform != nullptr)
            {
                float x_scaled = event->mousePosition.x * TESettings::VIEWPORT_SCALE;
                float y_scaled = event->mousePosition.y * TESettings::VIEWPORT_SCALE;
                glm::vec3 t_pos = transform->get_position_from_camera();
                if (x_scaled > (t_pos.x - transform->width) && x_scaled < (t_pos.x + transform->width) &&
                    y_scaled > (t_pos.y - transform->height) && y_scaled < (t_pos.y + transform->height))
                {
                    if (event->eventType == "LeftMousePress")
                    {
                        notify_listener(event, entity, selected_index);
                    }
                    if (event->eventType == "RightMousePress")
                    {
                        notify_listener(event, entity, selected_layer);
                    }
                    found_entity = true;
                    break;
                }
            }
        }
        for (const auto &[entity, transform] : ui_transforms)
        {
            if (transform != nullptr)
            {
                float x_scaled = event->mousePosition.x * TESettings::VIEWPORT_SCALE;
                float y_scaled = event->mousePosition.y * TESettings::VIEWPORT_SCALE;
                glm::vec3 t_pos = transform->get_position_from_camera();
                if (event->mousePosition.x > (t_pos.x - transform->width) && event->mousePosition.x < (t_pos.x + transform->width) &&
                    event->mousePosition.y > (t_pos.y - transform->height) && event->mousePosition.y < (t_pos.y + transform->height))
                {
                    if (event->eventType == "LeftMousePress")
                    {
                        notify_listener(event, entity, selected_index);
                    }
                    if (event->eventType == "RightMousePress")
                    {
                        notify_listener(event, entity, selected_layer);
                    }
                    found_entity = true;
                    break;
                }
            }
        }
        if (!found_entity)
        {
            // if no entity was clicked, we treat it as a "global" click
            for (const auto &[entity, listener] : listeners)
            {
                event->eventType = "Global" + event->eventType;
                notify_listener(event, entity, 0);
            }
        }
    }
    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        // ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X * yoffset) * 0.5f;
        // ViewportSettings::SCR_VIEWPORT_Y += (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
    }
};