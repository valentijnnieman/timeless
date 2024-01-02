#pragma once

class Node
{
public:
    int x, y, z;
    int layer;
    int width, height;
    int index;

    Collider collider;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(x, y, z, width, height, index, layer);
    }
    Node() {}

    Node(int x, int y, int z, int width, int height, int index, int layer)
        : x(x), y(y), z(z), layer(layer), width(width), height(height), index(index)
    {
        float x1 = x - width;
        float x2 = x + width;
        float y1 = y - height;
        float y2 = y + height;

        collider = Collider(x1, x2, y1, y2, layer);
    }
};