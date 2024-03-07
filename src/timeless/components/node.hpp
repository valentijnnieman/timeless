#pragma once

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
};
std::string to_string(LocationType t)
{
	switch (t)
	{
	case none:    return "none";
	case home:    return "home";
	case office:  return "office";
	case food:    return "food";
	case walking_spot: return "walking_spot";
	case hangout: return "hangout";
	case crime: return "crime";
	case meeting: return "meeting";
	default:      return "unknown";
	}
}

LocationType loc_from_string(std::string t)
{
	if(t == "none") return none;
	if(t == "home") return home;
	if(t == "office")  return office;
	if(t == "food")    return food;
	if(t == "walking_spot") return walking_spot;
	if(t == "hangout") return hangout;
	if(t == "crime") return crime;
	if(t == "meeting") return meeting;
}


class Node
{
public:
	int x, y, z;
	int layer;
	int width, height;
	int index;
	bool inside;
	int taken = 0;

	std::vector<Entity> entities; // which entities are located on this node

	LocationType location = none;

	Collider collider;

	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(x, y, z, width, height, index, layer);
	}

	Node(int x, int y, int z, int width, int height, int index, int layer, bool inside = false, LocationType location = none)
		: x(x), y(y), z(z), layer(layer), width(width), height(height), index(index), inside(inside), location(location)
	{
		float x1 = x - width;
		float x2 = x + width;
		float y1 = y - height;
		float y2 = y + height;

		collider = Collider(x1, x2, y1, y2, layer);
	}

	void add_entity(Entity entity)
	{
		entities.push_back(entity);
	}
	void remove_entity(Entity entity)
	{
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
	int get_entity_index(Entity entity)
	{
		if (!entities.empty())
		{
			auto found = std::find_if(entities.begin(), entities.end(), [&](auto& e)
				{ return e == entity; });
			if (found != entities.end())
			{
				return found - entities.begin();
			}
		}
		return -1;
	}
	bool includes_entity(Entity entity)
	{
		if (!entities.empty())
		{
			auto found = std::find_if(entities.begin(), entities.end(), [&](auto& e)
				{ return e == entity; });
			if (found != entities.end())
			{
				return true;
			}
		}
		return false;
	}
};