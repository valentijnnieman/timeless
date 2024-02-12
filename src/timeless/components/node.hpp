#pragma once

enum LocationType 
{
	none,
	home,
	office,
	food,
    car_start,
    bike_start,
    crime,
    meeting,	
};
std::string to_string (LocationType t)
{
    switch (t)
    {
		case none:    return "none";
		case home:    return "home";
		case office:  return "office";
		case food:    return "food";
		case car_start: return "car_start";
		case bike_start: return "bike_start";
		case crime: return "crime";
		case meeting: return "meeting";
		default:      return "unknown";
    }
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
    LocationType location = none;

    Collider collider;

    template <class Archive>
    void serialize(Archive &archive)
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
};