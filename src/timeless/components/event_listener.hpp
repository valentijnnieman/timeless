#pragma once
#include <functional>
#include "../entity.hpp"
#include "../event.hpp"

template <typename T = Event>
class EventListener
{
private:
	std::function<void(T* event, Entity entity, void* data)> callback;
public:
	EventListener(std::function<void(T* event, Entity entity, void* data)> callback)
		: callback(callback)
	{
	}

	template <typename DT>
	void callback_handler(T* event, Entity entity, DT* data)
	{
		callback(event, entity, data);
	}
};

