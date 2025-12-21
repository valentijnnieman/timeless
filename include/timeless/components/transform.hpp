#pragma once
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

class Transform : public Component
{
private:
  std::queue<glm::vec3> positions;
  std::queue<glm::vec3> scales;
  std::queue<glm::vec3> rotations;

public:
    glm::vec3 camera_position = glm::vec3(0.0f);
    glm::mat4 model, view, projection;

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

    bool uses_3d_projection = false;

    Transform(glm::vec3 p, float r, float w, float h, glm::vec3 o = glm::vec3(0.0f), bool center = true, bool zoomable = true)
        : position(p), start_position(p),
          width(w), height(h), offset(o), center(center), scale(glm::vec3(1.0f)), zoomable(zoomable)
    {
        rotation = glm::quat(glm::vec3(0, 0, 0));
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
    }
    void translate(glm::vec3 p)
    {
        position += p;
    }
    void setRotation(const glm::quat& q) {
        rotation = q;
    }

    void setRotationEuler(const glm::vec3& eulerDegrees) {
        glm::vec3 radians = glm::radians(eulerDegrees);
        rotation = glm::quat(radians);
    }

    glm::quat getRotationQuat() const {
        return rotation;
    }

    glm::vec3 getRotationEuler() const {
        glm::vec3 radians = glm::eulerAngles(rotation);
        return glm::degrees(radians);
    }

    glm::mat4 getRotationMatrix() const {
        return glm::toMat4(rotation);
    }
    void set_position(glm::vec3 p)
    {
        position = p;
    }
    void set_offset(glm::vec3 o)
    {
        offset = o;
    }
    void set_scale(glm::vec3 s)
    {
        scale = s;
        // width = s.x;
        // height = s.y;
    }

    void update_camera(glm::vec3 p)
    {
        camera_position = p;
    }

    glm::vec3 get_position()
    {
      glm::vec3 p = position;
      if(!positions.empty()){
        p = positions.front();
        position = p;
        positions.pop();
        if (loop) {
          positions.push(p);
        }
      }
    return p;
    }

    glm::vec3 get_position_minus_offset()
    {
      glm::vec3 p = get_position();
      return glm::vec3((p.x - offset.x), (p.y - offset.y), p.z);
    }

    /* returns position minus camera - i.e. position as if looking from
    camera's perspective */
    glm::vec3 get_position_from_camera()
    {
      glm::vec3 p = get_position();
        return glm::vec3((p.x - offset.x) - camera_position.x, (p.y - offset.y) - camera_position.y, p.z - camera_position.z);
    }

    glm::vec3 get_centered_position_from_camera()
    {
      glm::vec3 p = get_position();
      glm::vec3 p_centered = glm::vec3((p.x - offset.x) - camera_position.x, (p.y - offset.y) - camera_position.y, p.z - camera_position.z) + glm::vec3(0.5 * width, 0.5 * height, 0.0);
      return p_centered / TESettings::VIEWPORT_SCALE;
    }

    void set_position_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      while (!positions.empty())
        positions.pop();

      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        positions.push(glm::vec3(x_l, y_l, z_l));
      }
    }

    void append_position_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        positions.push(glm::vec3(x_l, y_l, z_l));
      }
    }


    void set_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      while (!rotations.empty())
        rotations.pop();

      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        rotations.push(glm::vec3(x_l, y_l, z_l));
      }
    }

    void append_rotation_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        rotations.push(glm::vec3(x_l, y_l, z_l));
      }
    }

    void set_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      while (!scales.empty())
        scales.pop();

      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        scales.push(glm::vec3(x_l, y_l, z_l));
      }
    }
    void append_scale_frames(glm::vec3 from, glm::vec3 to, float speed = 1.0) {
      for (double i = 0.0; i <= 1.0; i += speed / 60.0) {
        double x_l = std::lerp(from.x, to.x, i);
        double y_l = std::lerp(from.y, to.y, i);
        double z_l = std::lerp(from.z, to.z, i);
        scales.push(glm::vec3(x_l, y_l, z_l));
      }
    }

    void update(int x = TESettings::VIEWPORT_X, int y = TESettings::VIEWPORT_Y, float zoom = 1.0f, glm::vec3 offset = glm::vec3(0.0f))
    {
      if (zoomable)
        projection =
            glm::ortho(-(static_cast<float>(x * 0.5) * zoom),
                       static_cast<float>(x * 0.5) * zoom,
                       (static_cast<float>(y * 0.5) * zoom),
                       -static_cast<float>(y * 0.5) * zoom, -1000.0f, 1000.0f);
      else
        projection =
            glm::ortho(-(static_cast<float>(x * 0.5) ),
                      static_cast<float>(x * 0.5) ,
                      (static_cast<float>(y * 0.5)),
                      -static_cast<float>(y * 0.5), -1000.0f, 1000.0f);

      view = glm::lookAt(camera_position, camera_position + glm::vec3(0, 0, -1),
                         glm::vec3(0, 1, 0));

      model = glm::mat4(1.0f);


      if(uses_3d_projection) {
        float aspect = static_cast<float>(x) / static_cast<float>(y);
        projection = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 100.0f);
        // model = glm::scale(model, glm::vec3(0.1f));
      }

      glm::mat4 modelTransform = glm::mat4(1.0f);
      modelTransform =
          glm::translate(modelTransform, get_position_minus_offset());

      if (center) {
        model =
            glm::translate(model, glm::vec3(0.5 * width, 0.5 * height, 0.0));
      } else {
        model = glm::translate(model, glm::vec3(0.5 * width, 0.0, 0.0));
      }
      model = glm::translate(model, offset);

      glm::mat4 rotation_matrix = getRotationMatrix();

      if(!rotations.empty()) {
        glm::vec3 new_r = rotations.front();
        auto eulers = glm::quat(new_r);
        glm::quat rot = eulers;
        rotation_matrix = glm::toMat4(eulers);
        rotations.pop();
        if (loop) {
          rotations.push(new_r);
        }
      }

      model = modelTransform * rotation_matrix * model;

      if (flip) {
        model = glm::rotate(model, glm::radians(180.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
      }

      model = glm::scale(model, glm::vec3(width, height, 1.0));
      glm::vec3 s = scale;
      if(!scales.empty()) {
        s = scales.front();
        scales.pop();
        if(!reset) {
          scale = s;
        }
        if (loop) {
          scales.push(scale);
        }
      }
      model = glm::scale(model, s);



    }
};
