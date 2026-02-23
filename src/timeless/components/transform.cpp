#include "timeless/components/transform.hpp"

Transform::Transform(glm::vec3 p, float r, float w, float h,
                     glm::vec3 o, bool center,
                     bool zoomable)
    : position(p), start_position(p), width(w), height(h), offset(o),
      center(center), scale(glm::vec3(1.0f)), zoomable(zoomable) {
  rotation = glm::quat(glm::vec3(0, 0, 0));
  model = glm::mat4(1.0f);
}
Transform::Transform(glm::vec3 p, glm::vec3 rot, glm::vec3 s,
                     glm::vec3 o, bool center,
                     bool zoomable)
    : position(p), start_position(p), width(0.0f), height(0.0f), offset(o),
      center(center), scale(s), zoomable(zoomable) {
  rotation = glm::quat(rot);
  model = glm::mat4(1.0f);
}
void Transform::translate(glm::vec3 p) { position += p; }
void Transform::setRotation(const glm::quat &q) { rotation = q; }

void Transform::setRotationEuler(const glm::vec3 &eulerDegrees) {
  glm::vec3 radians = glm::radians(eulerDegrees);
  rotation = glm::quat(radians);
}

void Transform::setRotationAngleAxis(float angleDegrees,
                                     const glm::vec3 &axis) {
  float angleRadians = glm::radians(angleDegrees);
  rotation = glm::angleAxis(angleRadians, glm::normalize(axis));
}

glm::quat Transform::getRotationQuat() const { return rotation; }

glm::vec3 Transform::getRotationEuler() const {
  glm::vec3 radians = glm::eulerAngles(rotation);
  return glm::degrees(radians);
}

glm::mat4 Transform::getRotationMatrix() {
  glm::mat4 rotation_matrix = glm::toMat4(rotation);
  if (!rotations.empty()) {
    glm::vec3 new_r = rotations.front();
    auto eulers = glm::quat(new_r);
    glm::quat rot = eulers;
    rotation_matrix = glm::toMat4(eulers);
    rotations.pop();
    if (loop) {
      rotations.push(new_r);
    }
  }
  return rotation_matrix;
}

void Transform::set_position(glm::vec3 p) { position = p; }
void Transform::set_offset(glm::vec3 o) { offset = o; }
void Transform::set_scale(glm::vec3 s) {
  scale = s;
  // width = s.x;
  // height = s.y;
}
glm::vec3 Transform::get_scale() {
  glm::vec3 s = scale;
  if (!scales.empty()) {
    s = scales.front();
    scales.pop();
    if (!reset) {
      scale = s;
    }
    if (loop) {
      scales.push(scale);
    }
  }
  return s;
}

void Transform::update_camera(std::shared_ptr<Camera> camera) {
  this->camera = camera;
  camera_position = camera->get_position();
  camera_rotation = camera->get_rotation();
}

glm::vec3 Transform::get_position() {
  glm::vec3 p = position;
  if (!positions.empty()) {
    p = positions.front();
    position = p;
    positions.pop();
    if (loop) {
      positions.push(p);
    }
  }
  return p;
}

glm::vec3 Transform::get_position_minus_offset() {
  glm::vec3 p = get_position();
  return glm::vec3((p.x - offset.x), (p.y - offset.y), p.z);
}

/* returns position minus camera - i.e. position as if looking from
camera's perspective */
glm::vec3 Transform::get_position_from_camera() {
  glm::vec3 p = get_position();
  glm::vec3 relative = p - camera->get_position();
  // Apply inverse camera rotation to get position in camera space
  // glm::vec3 camera_space = glm::inverse(glm::normalize(camera_rotation)) *
  // relative;
  return relative;
}

glm::vec3 Transform::get_centered_position_from_camera() {
  glm::vec3 p = get_position();
  glm::vec3 p_centered =
      glm::vec3((p.x - offset.x) - camera_position.x,
                (p.y - offset.y) - camera_position.y, p.z - camera_position.z) +
      glm::vec3(0.5 * width, 0.5 * height, 0.0);

  p_centered /= TESettings::VIEWPORT_SCALE;
  // Apply inverse camera rotation to get position in camera space
  // glm::vec3 camera_space = glm::inverse(glm::normalize(camera_rotation)) *
  // p_centered;

  return p_centered;
}

void Transform::set_position_frames(glm::vec3 from, glm::vec3 to,
                                    float speed) {
  while (!positions.empty())
    positions.pop();

  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    positions.push(glm::vec3(x_l, y_l, z_l));
  }
}

void Transform::append_position_frames(glm::vec3 from, glm::vec3 to,
                                       float speed) {
  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    positions.push(glm::vec3(x_l, y_l, z_l));
  }
}

void Transform::set_rotation_frames(glm::vec3 from, glm::vec3 to,
                                    float speed) {
  while (!rotations.empty())
    rotations.pop();

  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    rotations.push(glm::vec3(x_l, y_l, z_l));
  }
}

void Transform::append_rotation_frames(glm::vec3 from, glm::vec3 to,
                                       float speed) {
  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    rotations.push(glm::vec3(x_l, y_l, z_l));
  }
}

void Transform::set_scale_frames(glm::vec3 from, glm::vec3 to,
                                 float speed) {
  while (!scales.empty())
    scales.pop();

  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    scales.push(glm::vec3(x_l, y_l, z_l));
  }
}
void Transform::append_scale_frames(glm::vec3 from, glm::vec3 to,
                                    float speed) {
  for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
    double x_l = std::lerp(from.x, to.x, i);
    double y_l = std::lerp(from.y, to.y, i);
    double z_l = std::lerp(from.z, to.z, i);
    scales.push(glm::vec3(x_l, y_l, z_l));
  }
}

BoundingSphere Transform::get_bounding_sphere() const {
  // Transform local center to world space
  glm::vec3 world_center =
      glm::vec3(model * glm::vec4(local_bounding_center, 1.0f));
  // Scale radius by the largest scale component
  float max_scale = glm::compMax(glm::abs(scale));
  float world_radius = local_bounding_radius * max_scale;
  return {world_center, world_radius};
}
