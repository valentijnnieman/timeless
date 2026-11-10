#pragma once
#include "timeless/components/collider.hpp"
#include "timeless/components/node.hpp"
#include "timeless/entity.hpp"
#include <glm/glm.hpp>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace std {
template <> struct hash<Node> {
  std::size_t operator()(const Node &id) const noexcept {
    return std::hash<int>()(id.x ^ (id.y << 16));
  }
};
} // namespace std
inline bool operator==(const Node &lhs, const Node &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
class Compare {
public:
  bool operator()(std::shared_ptr<Node> below, std::shared_ptr<Node> above) {
    if (below->layer > above->layer) {
      return true;
    }

    return false;
  }
};

class Grid {
private:
  typedef std::unordered_map<std::shared_ptr<Node>,
                             std::vector<std::shared_ptr<Node>>>
      map_type;
  typedef typename map_type::value_type map_value_type;

  int x_bounds = 400, y_bounds = 400;

  const std::vector<glm::vec2> directions = {
      /** N, E, S, W*/
      glm::vec2(0.0f, -1.0f),
      glm::vec2(1.0f, 0.0f),
      glm::vec2(0.0f, 1.0f),
      glm::vec2(-1.0f, 0.0f),
  };

public:
  map_type vertices;
  std::map<std::pair<int, int>, std::shared_ptr<Node>> nodes_by_pos;

  Grid() {}

  void purge() {
    vertices.clear();
    nodes_by_pos.clear();
  }

  // void register_entity(Entity entity) { registered_entities.push_back(entity); }
  bool isInBounds(glm::vec2 p) const {
    return 0 <= p.x && p.x < x_bounds && 0 <= p.y && p.y < y_bounds;
  }

  void calculate_nodes(std::map<Entity, std::shared_ptr<Node>> &nodes);

  void add_node(std::shared_ptr<Node> node,
                std::map<Entity, std::shared_ptr<Node>> &nodes);

  std::vector<std::shared_ptr<Node>>
  find_neighbours(std::shared_ptr<Node> node,
                  std::map<std::pair<int, int>, std::shared_ptr<Node>> &nodes);

  int detectGridLayer(Collider &at);

  std::shared_ptr<Node> find_node(int x, int y);

  // std::shared_ptr<Node> find_free_node_of_type(LocationType location)
  // {
  // 	auto it = std::find_if(vertices.begin(), vertices.end(),
  // [&](map_value_type node) 		{ return node.first->location == location &&
  // node.first->taken < 3; }); 	return it->first;
  // }

  int cost(std::shared_ptr<Node> current, std::shared_ptr<Node> next);

  typedef std::pair<int, std::shared_ptr<Node>> WeightedNode;
  std::vector<std::shared_ptr<Node>>
  get_path_to_node(std::shared_ptr<Node> start, std::shared_ptr<Node> dest);

  glm::vec3 calculate_directions(std::shared_ptr<Node> start,
                                 std::shared_ptr<Node> dest) {
    return glm::vec3(dest->x - start->x, dest->y - start->y, 1.0f);
  }
};
