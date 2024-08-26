#pragma once
#include "../event.hpp"
#include "system.hpp"

class EventSystem : public System
{
private:
	template <typename T>
	void notify_listener(ComponentManager& cm, Event* event, Entity entity, T* data)
	{
		try {
			auto listener = cm.get_event_listener(entity);
			listener->callback_handler<T>(event, entity, data);
		}
		catch (const std::exception& e) 
		{
		}
	}

	template <typename T>
	void notify_listener(ComponentManager& cm, PositionEvent* event, Entity entity, T* data)
	{
		try {
			auto listener = cm.get_position_event_listener(entity);
			listener->callback_handler<T>(event, entity, data);
		}
		catch (const std::exception& e) 
		{
		}
	}

public:
	EventSystem() {};

	template <typename T>
	bool create_event(ComponentManager& cm, const std::string& event_type, T* data)
	{
		Event* event = new Event(event_type);
		for (const auto& entity : registered_entities)
		{
			notify_listener<T>(cm, event, entity, data);
		}
		if (event->picked_up)
		{
			delete event;
			return true;
		}
		delete event;
		return false;
	}

	template <typename T>
	bool create_position_event(ComponentManager& cm, const std::string& event_type, glm::vec2 position, T* data)
	{
		PositionEvent* event = new PositionEvent(event_type, position);
		for (const auto& entity : registered_entities)
		{
			notify_listener<T>(cm, event, entity, data);
		}
		if (event->picked_up)
		{
			delete event;
			return true;
		}
		delete event;
		return false;
	}
};
