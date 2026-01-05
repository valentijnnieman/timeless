#pragma once
#include "glm/glm.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/shader.hpp"
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

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Shader> shader, glm::vec3 diffuseColor, glm::vec3 specularColor);

  void setupMesh(std::shared_ptr<Shader> shader);

  unsigned int VAO, VBO, EBO;
private:
};
