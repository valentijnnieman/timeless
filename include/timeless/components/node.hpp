#pragma once
#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "timeless/components/collider.hpp"
#include "timeless/components/component.hpp"
#include "timeless/entity.hpp"
#include "timeless/mutex.hpp"

enum LocationType
{
	none,
	home,
	office,
	food,
	walking_spot,
	hangout,
	crime,
	meeting,
	rendezvous
};
const int LOCATIONS_AMOUNT = 9;

std::string to_string(LocationType t);

LocationType loc_from_string(std::string t);

inline std::vector<glm::vec2> directions = {
	/** N, E, S, W*/
	glm::vec2(0.0f, -1.0f),
	glm::vec2(1.0f, 0.0f),
	glm::vec2(0.0f, 1.0f),
	glm::vec2(-1.0f, 0.0f),
};
enum dirs
{
	north,
	east,
	south,
	west
};



class Node : public Component
{
private:
  Mutex entities_mutex;

public:
	int x, y, z;
	int layer;
	int width, height;
	int index;
	bool inside;
	int taken = 0;

	// which neighbouring nodes are allowed 
	std::vector<glm::vec2> entrances;
	std::vector<glm::vec2> exits;

	std::set<Entity> entities; // which entities are located on this node
	std::set<int> extra_data; // extra data located on this node

	LocationType location = none;
  bool shrouded = false;

	Collider collider;

	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(x, y, z, width, height, index, layer);
	}

	Node(int x, int y, int z, int width, int height, int index, int layer, bool inside = false, LocationType location = none, std::vector<glm::vec2> entrances = {directions[north], directions[east], directions[south], directions[west]}, std::vector<glm::vec2> exits = {directions[north], directions[east], directions[south], directions[west]});

	void add_data(int data)
	{
		extra_data.insert(data);
	}
	void remove_data(int data);
	int get_data_index(int index);
	void add_entity(Entity entity)
	{
    std::lock_guard lock(entities_mutex);
		entities.insert(entity);
	}
	void remove_entity(Entity entity)
	{
    std::lock_guard lock(entities_mutex);
		if (!entities.empty())
		{
			auto found = std::find_if(entities.begin(), entities.end(), [&](auto& e)
				{ return e == entity; });
			if (found != entities.end())
			{
				entities.erase(found);
			}
		}
	}
int get_entity_index(const Entity& entity) const;
	bool includes_entity(Entity entity);
};
