#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/string_cast.hpp"
#include "timeless/components/component.hpp"
#include <memory>
#include <queue>
#include <optional>

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

  glm::vec3 get_position() {
    if (positions.size() > 0) {
      glm::vec3 p = positions.front();
      positions.pop();
      return p;
    }
    return position;
  }
  void set_position(glm::vec3 pos) { 
    position = pos; 
  }

  glm::quat get_rotation() { return rotation; }
  void set_rotation(glm::quat rot) { 
    rotation = rot; 
  }

  // Returns the camera's local forward vector (direction it's looking)
  glm::vec3 get_forward() const {
    return rotation * forward;
  }

  // Returns the camera's local up vector
  glm::vec3 get_up() const {
    return rotation * up;
  }

  // Returns the camera's local rightG vector
  glm::vec3 get_right() const {
    // Right is cross(forward, up)
    return glm::normalize(glm::cross(get_forward(), get_up()));
  }

  glm::mat4 get_view_matrix() {
    if(perspective) {
      if (focus_point) {
        view = glm::lookAt(position, *focus_point + get_forward(), get_up());
      } else {
        view = glm::lookAt(position, position + get_forward(),
                          get_up());
      }
    } else {
      if (focus_point) {
        view = glm::lookAt(position, *focus_point + get_forward(), get_up());
      } else {
        view = glm::lookAt(position, position + get_forward(),
                          get_up());
      }
    }
    return view;
  }

  glm::mat4 get_projection_matrix(int x = TESettings::VIEWPORT_X, int y = TESettings::VIEWPORT_Y, float zoom = 1.0f) {
      if (perspective) {
        projection = glm::perspective(glm::radians(40.0f),
                                      float(x) / float(y), 1.0f, 10000.0f);
        projection *= glm::scale(glm::mat4(1.0f), glm::vec3(-1, 1, 1));
        // Flip Y axis to match ortho orientation
        // projection[1][1] *= -1;
      } else {
        projection = glm::ortho(-(static_cast<float>(x * 0.5) * zoom),
                                static_cast<float>(x * 0.5) * zoom,
                                -(static_cast<float>(y * 0.5) * zoom),
                                static_cast<float>(y * 0.5) * zoom, -10000.0f,
                                10000.0f);
      }

    return projection;
  }

  void focus_on_position(const glm::vec3& pos) {
    glm::vec3 forward = this->get_forward();

    // Project the vector from camera to tile onto the forward vector
    float forward_offset = glm::dot(this->get_position() - pos, forward);

    // Set new camera position so tile is centered in view
    glm::vec3 new_camera_position = pos + forward * forward_offset;

    // this->set_positions_from_to(this->get_position(), new_camera_position, 0.5);
    this->animate_to(new_camera_position, rotation, 0.1);
  }

  // Start animation for position and rotation
  void animate_to(glm::vec3 toPos, glm::quat toRot, double duration) {
    startPos = position;
    endPos = toPos;
    startRot = rotation;
    endRot = toRot;
    animTime = 0.0;
    animDuration = duration;
    animating = true;
  }

  // Call this every frame with delta time
  void update(double dt) {
    if (animating) {
      animTime += dt;
      double t = glm::clamp(animTime / animDuration, 0.0, 1.0);
      position = glm::mix(startPos, endPos, t);
      rotation = glm::slerp(startRot, endRot, static_cast<float>(t));
      if (t >= 1.0) animating = false;
    }
  }

  // Existing position queue-based movement
  void set_positions_from_to(glm::vec3 from, glm::vec3 to, double speed) {
    for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
      double x_l = std::lerp(from.x, to.x, i);
      double y_l = std::lerp(from.y, to.y, i);
      double z_l = std::lerp(from.z, to.z, i);
      positions.push(glm::vec3(x_l, y_l, z_l));
    }
    position = to;
  }

  void animate_circle_around_center(const glm::vec3& center, float radius, float start_angle, float end_angle, float duration) {
    // Store orbit parameters as member variables if needed
    orbit_center = center;
    orbit_radius = radius;
    orbit_angle = start_angle;
    orbit_end_angle = end_angle;
    orbit_duration = duration;
    orbit_elapsed = 0.0f;
    orbit_angular_speed = (end_angle - start_angle) / duration;
    orbit_active = true;
  }

void update_orbit(float dt) {
  if (!orbit_active) return;
  orbit_elapsed += dt;
  if (orbit_elapsed > orbit_duration) {
    orbit_angle = orbit_end_angle;
    orbit_active = false;
  } else {
    orbit_angle += orbit_angular_speed * dt;
  }
  glm::vec3 offset = glm::vec3(
    orbit_radius * cos(orbit_angle),
    orbit_radius * sin(orbit_angle),
    1000.0f
  );
  position = orbit_center + offset;

  // Compute yaw to face the center (Z up)
  glm::vec3 to_center = glm::normalize(orbit_center - position);
  float yaw = atan2(to_center.y, to_center.x);

  // Use stored original downward pitch
  float pitch = 45.0f; // Store this when starting the orbit

  // Compose rotation: yaw (Z axis), then pitch (X axis)
  glm::quat q_yaw = glm::angleAxis(yaw, glm::vec3(0, 0, 1));
  glm::quat q_pitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
  rotation = q_yaw * q_pitch;
}
};
