#pragma once
#include "timeless/event.hpp"
#include "timeless/systems/system.hpp"
#include "timeless/components/mouse_input_listener.hpp"

class MouseInputSystem : public System
{
public:
	Entity camera;

	void register_camera(Entity c)
	{
		camera = c;
	}
	std::vector<Entity> registered_move_entities;

	void register_move_entity(Entity entity)
	{
		registered_move_entities.push_back(entity);
	}
	void remove_move_entity(Entity entity)
	{
		if (!registered_move_entities.empty())
		{
			auto found = std::find_if(registered_move_entities.begin(), registered_move_entities.end(), [&](auto& e)
				{ return e == entity; });
			if (found != registered_move_entities.end())
			{
				registered_move_entities.erase(found);
			}
		}
	}

	void notify_listener(ComponentManager& cm, MouseEvent* event, Entity entity)
	{
		auto listener = cm.get_component<MouseInputListener<MouseEvent>>(entity);
    if(listener != nullptr)
      listener->on_click_handler(event, entity, 0);
	}

	void notify_listener(ComponentManager& cm, MouseMoveEvent* event, Entity entity)
	{
		auto listener = cm.get_component<MouseInputListener<MouseMoveEvent>>(entity);
    if(listener != nullptr)
      listener->on_click_handler(event, entity, 0);
	}

	void mouse_click_handler(ComponentManager& cm, MouseEvent* event)
	{
		auto cam = cm.get_component<Camera>(camera);

		std::string et = event->eventType;
		event->eventType = "Global" + et;
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
		delete event;
	}
	void mouse_release_handler(ComponentManager& cm, MouseEvent* event)
	{
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
		}
		delete event;
	}
	void mouse_move_handler(ComponentManager& cm, MouseMoveEvent* event)
	{
		for (const auto& entity : this->registered_move_entities)
		{
			notify_listener(cm, event, entity);
      if(event->picked_up) {
        break;
      }
		}
		delete event;
	}
	void mouse_scroll_handler(ComponentManager& cm, MouseEvent* event)
	{
		for (const auto& entity : registered_entities)
		{
			notify_listener(cm, event, entity);
      if(event->picked_up) {
        break;
      }
		}
		delete event;
	}
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		// ViewportSettings::SCR_VIEWPORT_X += (ViewportSettings::SCR_VIEWPORT_X * yoffset) * 0.5f;
		// ViewportSettings::SCR_VIEWPORT_Y += (ViewportSettings::SCR_VIEWPORT_Y * yoffset) * 0.5f;
	}
};
