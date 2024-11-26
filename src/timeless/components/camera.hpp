#pragma once
#include "glm/glm.hpp"
#include "component.hpp"
#include <queue>

class Camera : public Component {
private:
  std::queue<glm::vec3> positions;
  glm::vec3 position;

public:
  Camera(glm::vec3 p) : position(p) {}

  glm::vec3 get_position() {
    if (positions.size() > 0) {
      glm::vec3 p = positions.front();
      positions.pop();
      return p;
    }
    return position;
  }
  void set_position(glm::vec3 pos) { position = pos; }

  void set_positions_from_to(glm::vec3 from, glm::vec3 to, double speed) {
    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from.x, to.x, i);
      double y_l = std::lerp(from.y, to.y, i);
      double z_l = std::lerp(from.z, to.z, i);
      positions.push(glm::vec3(x_l, y_l, z_l));
    }
    position = to;
  }
};
