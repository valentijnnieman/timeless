#pragma once
#include "glm/glm.hpp"
#include "timeless/components/component.hpp"
#include "timeless/components/shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>

struct VertexBoneData {
  float ids[4] = {0.0f};    // Indices of influencing bones (max 4)
  float weights[4] = {0.0f};    // Corresponding weights
};

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
  VertexBoneData boneData;
};

struct BoneInfo {
  glm::mat4 offsetMatrix; // Inverse bind pose matrix for this bone
};

class Mesh : public Component {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<BoneInfo> boneInfos;
  std::unordered_map<std::string, unsigned int> boneMapping; // maps a bone name to its index
  //
  glm::vec3 diffuseColor;
  glm::vec3 specularColor;

  // Node transform from the FBX scene hierarchy (accumulated from root to this mesh's node)
  glm::mat4 nodeTransform = glm::mat4(1.0f);
  // Parent's global transform — needed to recompute nodeTransform from animated local transforms
  glm::mat4 nodeParentTransform = glm::mat4(1.0f);
  std::string nodeName;

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

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<BoneInfo> boneInfos,
       std::unordered_map<std::string, unsigned int> boneMapping,
       std::shared_ptr<Shader> shader, glm::vec3 diffuseColor,
       glm::vec3 specularColor);

  void setupMesh(std::shared_ptr<Shader> shader);

  glm::mat4 getModelMatrix() {
    glm::mat4 local = glm::mat4(1.0f);
    local = glm::translate(local, position);
    local = glm::rotate(local, glm::radians(rotation.x),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    local = glm::rotate(local, glm::radians(rotation.y),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    local = glm::rotate(local, glm::radians(rotation.z),
                        glm::vec3(0.0f, 0.0f, 1.0f));
    local = glm::scale(local, scale);
    return local;
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
