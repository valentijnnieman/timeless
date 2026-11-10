#include "timeless/components/node.hpp"
#include <algorithm>
#include <iterator>

std::string to_string(LocationType t) {
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
	case rendezvous: return "rendezvous";
	default:      return "unknown";
	}
}

LocationType loc_from_string(std::string t) {
	if(t == "none") return none;
	if(t == "home") return home;
	if(t == "office")  return office;
	if(t == "food")    return food;
	if(t == "walking_spot") return walking_spot;
	if(t == "hangout") return hangout;
	if(t == "crime") return crime;
	if (t == "meeting") return meeting;
	if (t == "rendezvous") return rendezvous;
	else return none;
}

Node::Node(int x, int y, int z, int width, int height, int index, int layer, bool inside, LocationType location, std::vector<glm::vec2> entrances, std::vector<glm::vec2> exits)
	: x(x), y(y), z(z), layer(layer), width(width), height(height), index(index), inside(inside), location(location), entrances(entrances), exits(exits) {
	float x1 = x - width;
	float x2 = x + width;
	float y1 = y - height;
	float y2 = y + height;

	collider = Collider(x1, x2, y1, y2, layer);
}

void Node::remove_data(int data) {
	if (!extra_data.empty())
	{
		auto found = std::find_if(extra_data.begin(), extra_data.end(), [&](auto& e)
			{ return e == data; });
		if (found != extra_data.end())
		{
			extra_data.erase(found);
		}
	}
}

int Node::get_data_index(int index) {
	if (!extra_data.empty())
	{
		auto found = std::find_if(extra_data.begin(), extra_data.end(), [&](auto& e)
			{ return e == index; });
		if (found != extra_data.end())
		{
        return std::distance(extra_data.begin(), found);
			// return found - entities.begin();
		}
	}
	return -1;
}

int Node::get_entity_index(const Entity& entity) const {
    auto found = std::find(entities.begin(), entities.end(), entity);
    if (found != entities.end()) {
      return static_cast<int>(std::distance(entities.begin(), found));
    }
    return -1;
}

bool Node::includes_entity(Entity entity) {
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
