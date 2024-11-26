#pragma once
#include <functional>
#include "component.hpp"
#include "../entity.hpp"
#include "../event.hpp"

template <typename T = Event, typename CDT = void*>
class EventListener : public Component
{
private:
	std::function<void(T* event, Entity entity, CDT data)> callback;

public:
	EventListener(std::function<void(T* event, Entity entity, CDT data)> callback)
		: callback(callback)
	{
	}

	template <typename DT>
	void callback_handler(T* event, Entity entity, DT* data)
	{
		callback(event, entity, data);
	}
};

