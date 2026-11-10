#include "timeless/algorithm/graph.hpp"
#include <algorithm>
#include <functional>
#include <queue>

void Grid::calculate_nodes(std::map<Entity, std::shared_ptr<Node>> &nodes) {
  for (const auto &[entity, node] : nodes) {
    add_node(node, nodes);
  }

  for (const auto &[entity, node] : nodes) {
    std::vector<std::shared_ptr<Node>> neighbours =
        find_neighbours(node, nodes_by_pos);
    vertices.insert(
        std::pair<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>>(
            node, neighbours));
  }
}

void Grid::add_node(std::shared_ptr<Node> node,
                    std::map<Entity, std::shared_ptr<Node>> &nodes) {
  nodes_by_pos.insert({{node->x, node->y}, node});
}

std::vector<std::shared_ptr<Node>>
Grid::find_neighbours(std::shared_ptr<Node> node,
                      std::map<std::pair<int, int>, std::shared_ptr<Node>> &nodes) {
  std::vector<std::shared_ptr<Node>> results;
  std::vector<glm::vec2> node_dirs = directions;
  if (node->layer == 1)
    return results;

  if (node->exits.size() > 0) {
    node_dirs = node->exits;
  }
  for (const glm::vec2 &dir : node_dirs) {
    glm::vec2 next = {node->x + dir.x, node->y + dir.y};
    // if (isInBounds(next)) {
      auto it = nodes.find({next.x, next.y});
      if (it != nodes.end()) {
        const auto n = it->second;
        if (n->layer != 1) {
          std::vector<glm::vec2> n_dirs = directions;
          // if (n->index > 23 && n->index < 32)
          if (n->entrances.size() > 0) {
            n_dirs = n->entrances;
          }
          if (std::find(n_dirs.begin(), n_dirs.end(),
                        glm::vec2(dir.x, dir.y)) != n_dirs.end()) {
            results.push_back(n);
          }
        }
      // }
    }
  }

  return results;
}

int Grid::detectGridLayer(Collider &at) {
  auto it = std::find_if(vertices.begin(), vertices.end(),
                         [&at](const map_value_type &node) {
                           bool cX = (at.x1 >= node.first->collider.x1 ||
                                      at.x2 >= node.first->collider.x1) &&
                                     (at.x1 <= node.first->collider.x2 ||
                                      at.x2 <= node.first->collider.x2);

                           bool cY = (at.y1 >= node.first->collider.y1 ||
                                      at.y2 >= node.first->collider.y1) &&
                                     (at.y1 <= node.first->collider.y2 ||
                                      at.y2 <= node.first->collider.y2);

                           return cX && cY;
                         });

  if (it != vertices.end()) {
    return it->first->layer;
  } else
    return 0;
}

std::shared_ptr<Node> Grid::find_node(int x, int y) {
  auto it = std::find_if(vertices.begin(), vertices.end(),
                         [x = x, y = y](map_value_type node) {
                           return node.first->x == x && node.first->y == y;
                         });
  if (it == vertices.end()) {
      return nullptr;
  }
  return it->first;
}

int Grid::cost(std::shared_ptr<Node> current, std::shared_ptr<Node> next) {
  int c = (next->layer - current->layer) + 1;
  // Soft-avoid occupied cells (e.g. bods resting on towels): make crossing one
  // far costlier than a short detour, but still finite so a boxed-in entity can
  // path out rather than failing. taken is set/cleared by the rest lifecycle.
  if (next->taken > 0)
    c += 100;
  return c;
}

std::vector<std::shared_ptr<Node>>
Grid::get_path_to_node(std::shared_ptr<Node> start, std::shared_ptr<Node> dest) {
  std::priority_queue<WeightedNode, std::vector<WeightedNode>,
                      std::greater<WeightedNode>> frontier;
  frontier.emplace(0, start);

  std::unordered_map<std::shared_ptr<Node>, std::shared_ptr<Node>> came_from;
  std::unordered_map<std::shared_ptr<Node>, int> cost_so_far;

  came_from[start] = start;
  cost_so_far[start] = 0;

  while (!frontier.empty()) {
    auto current = frontier.top().second;
    frontier.pop();

    if (current == dest) {
      break;
    }
    for (const auto &next : vertices.at(current)) {
      int new_cost = cost_so_far[current] + cost(current, next);
      if (cost_so_far.find(next) == cost_so_far.end() ||
          new_cost < cost_so_far[next]) {
        cost_so_far[next] = new_cost;
        came_from[next] = current;
        frontier.emplace(new_cost, next);
      }
    }
  }

  auto current = dest;
  std::vector<std::shared_ptr<Node>> path;
  if (came_from.find(dest) == came_from.end()) {
    return path; // no path can be found
  }
  while (current != start) {
    path.push_back(current);
    current = came_from.at(current);
  }
  path.push_back(start);
  std::reverse(path.begin(), path.end());

  return path;
}
