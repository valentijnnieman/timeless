#pragma once
#include "timeless/entity.hpp"
#include "timeless/managers/component_manager.hpp"


class System
{
public:
	std::vector<Entity> registered_entities;

	virtual void register_entity(Entity entity)
	{
		registered_entities.push_back(entity);
	}
	virtual void remove_entity(Entity entity)
	{
		if (!registered_entities.empty())
		{
			auto found = std::find_if(registered_entities.begin(), registered_entities.end(), [&](auto& e)
				{ return e == entity; });
			if (found != registered_entities.end())
			{
				registered_entities.erase(found);
			}
		}
	}
	virtual void clear(ComponentManager &cm)
	{
		if (!registered_entities.empty())
		{
      for(auto ent : registered_entities) {
        cm.remove_entity(ent);
      }
      registered_entities.clear();
		}
	}
	virtual void update(ComponentManager &cm, GLFWwindow* window) {};
	virtual void update(ComponentManager &cm) {};
	virtual void render(ComponentManager& cm, int x, int y, float zoom = 1.0, int tick = 0) {};
	virtual void register_camera(Entity entity) {};
};
