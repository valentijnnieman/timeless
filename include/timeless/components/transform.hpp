#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <queue>
#include <iostream>
#include "timeless/settings.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/camera.hpp"

struct BoundingSphere {
    glm::vec3 center;
    float radius;
};

class Transform : public Component
{
private:
  std::queue<glm::vec3> positions;
  std::queue<glm::vec3> scales;
  std::queue<glm::vec3> rotations;

  glm::vec3 local_bounding_center = glm::vec3(0.0f); // Local-space center
  float local_bounding_radius = 1.0f;                // Local-space radius

public:
    std::shared_ptr<Camera> camera;
    glm::vec3 camera_position = glm::vec3(0.0f);
    glm::quat camera_rotation = glm::quat();
    glm::mat4 model;

    glm::mat4 light_view;
    glm::mat4 light_projection;
    glm::mat4 light_space;

    glm::vec3 position;
    glm::vec3 start_position;
    glm::vec3 offset;
    glm::vec3 scale;
    glm::quat rotation;

    float width, height;
    float grid_x = 0;
    float grid_y = 0;
    bool flip = false;
    bool center = true;
    bool zoomable = true;

    bool loop = false;
    bool triggered = false;
    bool reset = true;

    bool isometric = false;

    Transform(glm::vec3 p, float r, float w, float h, glm::vec3 o = glm::vec3(0.0f), bool center = true, bool zoomable = true);
    Transform(glm::vec3 p, glm::vec3 rot, glm::vec3 s, glm::vec3 o = glm::vec3(0.0f), bool center = true, bool zoomable = true);
    void translate(glm::vec3 p);
    void setRotation(const glm::quat& q);
    void setRotationEuler(const glm::vec3& eulerDegrees);

    void setRotationAngleAxis(float angleDegrees, const glm::vec3& axis);

    glm::quat getRotationQuat() const;

    glm::vec3 getRotationEuler() const;

    glm::mat4 getRotationMatrix();
    void set_position(glm::vec3 p);
    void set_offset(glm::vec3 o);
    void set_scale(glm::vec3 s);
    glm::vec3 get_scale();

    void update_camera(std::shared_ptr<Camera> camera);

    glm::vec3 get_position();

    glm::vec3 get_position_minus_offset();

    /* returns position minus camera - i.e. position as if looking from
    camera's perspective */
    glm::vec3 get_position_from_camera();

    glm::vec3 get_centered_position_from_camera();

    void set_position_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);

    void append_position_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);

    void set_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);

    void append_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);

    void set_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);
    void append_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0);
    BoundingSphere get_bounding_sphere() const;
};
