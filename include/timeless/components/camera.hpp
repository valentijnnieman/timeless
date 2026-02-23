#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/string_cast.hpp"
#include "timeless/components/component.hpp"
#include <memory>
#include <queue>
#include <optional>

struct Frustum {
    glm::vec4 planes[6]; // (a, b, c, d) for ax + by + cz + d = 0
};

class Camera : public Component {
private:
  std::queue<glm::vec3> positions;
  glm::vec3 position;
  glm::quat rotation;

  // Animation state
  glm::vec3 startPos, endPos;
  glm::quat startRot, endRot;
  double animTime = 0.0, animDuration = 0.0;
  bool animating = false;

  glm::mat4 view, projection;

  glm::vec3 forward, up;

  glm::vec3 orbit_center;
  float orbit_radius;
  float orbit_angle;
  float orbit_end_angle;
  float orbit_duration;
  float orbit_elapsed;
  float orbit_angular_speed;
  bool orbit_active = true;
public:
  bool perspective;
  std::optional<glm::vec3> focus_point;

  Camera(glm::vec3 p, bool perspective = false,
         glm::vec3 forward = glm::vec3(0.0f, -1.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f))
      : position(p), perspective(perspective), rotation(glm::quat()),
        forward(forward), up(up), view(glm::mat4(1.0f)) {}

  glm::vec3 get_position();
  void set_position(glm::vec3 pos);
  glm::quat get_rotation();
  void set_rotation(glm::quat rot);

  // Returns the camera's local forward vector (direction it's looking)
  glm::vec3 get_forward() const;
  // Returns the camera's local up vector
  glm::vec3 get_up() const;

  // Returns the camera's local rightG vector
  glm::vec3 get_right() const;

  glm::mat4 get_view_matrix();

  glm::mat4 get_projection_matrix(int x, int y, float zoom = 1.0f);

  Frustum get_frustum(int x, int y, float zoom = 1.0f);

  void focus_on_position(const glm::vec3& pos);
  // Start animation for position and rotation
  void animate_to(glm::vec3 toPos, glm::quat toRot, double duration);
  // Call this every frame with delta time
  void update(double dt);

  // Existing position queue-based movement
  void set_positions_from_to(glm::vec3 from, glm::vec3 to, double speed);
};
