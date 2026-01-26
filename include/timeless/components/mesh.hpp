#pragma once
#include "glm/glm.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <vector>

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

class Mesh : public Component {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  glm::vec3 diffuseColor;
  glm::vec3 specularColor;

  // Transform properties
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  // Animation state
  glm::vec3 startPos, endPos;
  float animTime = 0.0f, animDuration = 0.0f;
  bool animating = false;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::shared_ptr<Shader> shader, glm::vec3 diffuseColor,
       glm::vec3 specularColor);

  void setupMesh(std::shared_ptr<Shader> shader);

  glm::mat4 getModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z),
                        glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
  }

  void animate_to(const glm::vec3 &toPos, float duration) {
    startPos = position;
    endPos = toPos;
    animTime = 0.0f;
    animDuration = duration;
    animating = true;
  }

  void update_animation(float dt) {
    if (animating) {
      animTime += dt;
      float t = glm::clamp(animTime / animDuration, 0.0f, 1.0f);
      position = glm::mix(startPos, endPos, t);
      if (t >= 1.0f)
        animating = false;
    }
  }

  unsigned int VAO, VBO, EBO;

private:
};
