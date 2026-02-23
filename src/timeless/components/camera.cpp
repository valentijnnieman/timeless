#include "timeless/components/camera.hpp"

glm::vec3 Camera::get_position() {
  // if (positions.size() > 0) {
  //   glm::vec3 p = positions.front();
  //   positions.pop();
  //   return p;
  // }
  return position;
}
void Camera::set_position(glm::vec3 pos) { position = pos; }

glm::quat Camera::get_rotation() { return rotation; }
void Camera::set_rotation(glm::quat rot) {
  rotation = rot;
}

// Returns the camera's local forward vector (direction it's looking)
glm::vec3 Camera::get_forward() const { return rotation * forward; }

// Returns the camera's local up vector
glm::vec3 Camera::get_up() const { return rotation * up; }

// Returns the camera's local rightG vector
glm::vec3 Camera::get_right() const {
  // Right is cross(forward, up)
  return glm::normalize(glm::cross(get_forward(), get_up()));
}

glm::mat4 Camera::get_view_matrix() {
  if (perspective) {
    if (focus_point) {
      view = glm::lookAt(position, *focus_point + get_forward(), get_up());
    } else {
      view = glm::lookAt(position, position + get_forward(), get_up());
    }
  } else {
    if (focus_point) {
      view = glm::lookAt(position, *focus_point + get_forward(), get_up());
    } else {
      view = glm::lookAt(position, position + get_forward(), get_up());
    }
  }
  return view;
}

glm::mat4 Camera::get_projection_matrix(int x, int y, float zoom) {
  if (perspective) {
    projection = glm::perspective(glm::radians(40.0f), float(x) / float(y),
                                  1.0f, 10000.0f);
    projection *= glm::scale(glm::mat4(1.0f), glm::vec3(-1, 1, 1));
    // Flip Y axis to match ortho orientation
    // projection[1][1] *= -1;
  } else {
    projection =
        glm::ortho(static_cast<float>(x * 0.5) * zoom,
                   -(static_cast<float>(x * 0.5) * zoom),
                   -(static_cast<float>(y * 0.5) * zoom),
                   static_cast<float>(y * 0.5) * zoom, -10000.0f, 10000.0f);
  }

  return projection;
}

Frustum Camera::get_frustum(int x, int y, float zoom) {
  glm::mat4 vp = get_projection_matrix(x, y, zoom) * get_view_matrix();
  Frustum frustum;

  // Left
  frustum.planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0],
                                vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]);
  // Right
  frustum.planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0],
                                vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]);
  // Bottom
  frustum.planes[2] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1],
                                vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]);
  // Top
  frustum.planes[3] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1],
                                vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]);
  // Near
  frustum.planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2],
                                vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]);
  // Far
  frustum.planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2],
                                vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]);

  // Normalize planes
  for (int i = 0; i < 6; ++i) {
    float length = glm::length(glm::vec3(frustum.planes[i]));
    frustum.planes[i] /= length;
  }

  return frustum;
}

void Camera::focus_on_position(const glm::vec3 &pos) {
  glm::vec3 forward = this->get_forward();

  // Project the vector from camera to tile onto the forward vector
  float forward_offset = glm::dot(this->get_position() - pos, forward);

  // Set new camera position so tile is centered in view
  glm::vec3 new_camera_position = pos + forward * forward_offset;

  // this->set_positions_from_to(this->get_position(), new_camera_position,
  // 0.5);
  this->animate_to(new_camera_position, rotation, 0.1);
}

// Start animation for position and rotation
void Camera::animate_to(glm::vec3 toPos, glm::quat toRot, double duration) {
  startPos = position;
  endPos = toPos;
  startRot = rotation;
  endRot = toRot;
  animTime = 0.0;
  animDuration = duration;
  animating = true;
}

// Call this every frame with delta time
void Camera::update(double dt) {
  if (animating) {
    animTime += dt;
    double t = glm::clamp(animTime / animDuration, 0.0, 1.0);
    position = glm::mix(startPos, endPos, t);
    rotation = glm::slerp(startRot, endRot, static_cast<float>(t));
    if (t >= 1.0)
      animating = false;
  }
}

// Existing position queue-based movement
void Camera::set_positions_from_to(glm::vec3 from, glm::vec3 to, double speed) {
  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    positions.push(glm::vec3(x_l, y_l, z_l));
  }
  position = to;
}
