#pragma once
#include "glm/glm.hpp"
#include <unordered_map>
#include <queue>
#include <set>
#include <algorithm>
#include "../components/collider.hpp"
#include "../components/node.hpp"

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

class Graph
{
private:
    typedef std::unordered_map<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>> map_type;
    typedef typename map_type::value_type map_value_type;

    int x_bounds = 80, y_bounds = 80;
    std::array<glm::vec2, 4> directions = {
        /** N, E, S, W*/
        glm::vec2(0.0f, -1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(-1.0f, 0.0f),
    };

public:
    map_type vertices;
    std::vector<Entity> registered_entities;
    std::vector<NodeCollider> colliders;

    Graph() {}

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(vertices);
    }

    void register_entity(Entity entity)
    {
        registered_entities.push_back(entity);
    }
    bool isInBounds(glm::vec2 p) const
    {
        return 0 <= p.x && p.x < x_bounds && 0 <= p.y && p.y < y_bounds;
    }

    void is_inside(std::shared_ptr<Node> node)
    {
        for (auto &col : colliders)
        {
            if (col.is_in_bounds(glm::vec3(node->x, node->y, 0.0f)))
            {
                node->z = 3;
            }
        }
    }

    void add_node(std::shared_ptr<Node> node)
    {
        is_inside(node);

        vertices.insert(std::pair<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>>(node, {}));
    }

    void add_neighbour(std::shared_ptr<Node> node, std::shared_ptr<Node> to)
    {
        auto &v = vertices.at(node);
        if (std::find(v.begin(), v.end(), to) == v.end())
        {
            v.push_back(to);
        }
    }

    void connect_node_to(std::shared_ptr<Node> node, std::shared_ptr<Node> to)
    {
        add_neighbour(node, to);
        add_neighbour(to, node);
    }

    int detectGridLayer(Collider &at)
    {
        auto it = find_if(vertices.begin(), vertices.end(), [&at](const map_value_type &node)
                          {
            bool cX = (at.x1 >= node.first->collider.x1 ||
                       at.x2 >= node.first->collider.x1) &&
                      (at.x1 <= node.first->collider.x2 ||
                       at.x2 <= node.first->collider.x2);

            bool cY = (at.y1 >= node.first->collider.y1 ||
                       at.y2 >= node.first->collider.y1) &&
                      (at.y1 <= node.first->collider.y2 ||
                       at.y2 <= node.first->collider.y2);

            return cX && cY; });

        if (it != vertices.end())
        {
            return it->first->layer;
        }
        else
            return 0;
    }

    std::shared_ptr<Node> find_node(int x, int y)
    {
        auto it = find_if(vertices.begin(), vertices.end(), [&](map_value_type node)
                          { return node.first->x == x && node.first->y == y; });
        return it->first;
    }
    std::shared_ptr<Node> find_node_by_index(int index)
    {
        auto it = vertices.begin();
        std::advance(it, index);
        return it->first;
    }

    int cost(std::shared_ptr<Node> current, std::shared_ptr<Node> next)
    {
        return next->layer - current->layer;
    }

    std::vector<std::shared_ptr<Node>> get_path_to_node(std::shared_ptr<Node> start, std::shared_ptr<Node> dest)
    {
        typedef std::pair<int, std::shared_ptr<Node>> WeightedNode;
        std::priority_queue<WeightedNode, std::vector<WeightedNode>, std::greater<WeightedNode>> frontier;
        frontier.emplace(0, start);

        std::unordered_map<std::shared_ptr<Node>, std::shared_ptr<Node>> came_from;
        std::unordered_map<std::shared_ptr<Node>, int> cost_so_far;

        came_from[start] = start;
        cost_so_far[start] = 0;

        while (!frontier.empty())
        {
            auto current = frontier.top().second;
            frontier.pop();

            if (current == dest)
            {
                break;
            }
            for (auto next : vertices.at(current))
            {
                int new_cost = cost_so_far[current] + cost(current, next);
                if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
                {
                    cost_so_far[next] = new_cost;
                    came_from[next] = current;
                    frontier.emplace(new_cost, next);
                }
            }
        }

        auto current = dest;
        std::vector<std::shared_ptr<Node>> path;
        if (came_from.find(dest) == came_from.end())
        {
            return path; // no path can be found
        }
        while (current != start)
        {
            path.push_back(current);
            current = came_from.at(current);
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());

        return path;
    }

    glm::vec3 calculate_directions(std::shared_ptr<Node> start, std::shared_ptr<Node> dest)
    {
        return glm::vec3(dest->x - start->x, dest->y - start->y, 1.0f);
    }
};
