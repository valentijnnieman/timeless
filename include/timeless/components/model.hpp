#pragma once
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "timeless/components/component.hpp"
#include "timeless/components/mesh.hpp"
#include "timeless/components/texture.hpp"
#include <memory>
#include <map>
#include <unordered_map>

struct SkeletonBone {
    std::string name;
    int parentIndex; // -1 if root
    std::vector<int> childrenIndices;
};

class Model : public Component {
public:
  Model(const std::string &path, std::shared_ptr<Texture> texture,
        std::shared_ptr<Shader> shader)
      : texture(texture), shader(shader) {
    loadModel(path);
  }
  Model(const std::string &path, std::shared_ptr<Shader> shader)
      : shader(shader) {
    loadModel(path);
  }
  ~Model() {
    for (auto &mesh : meshes) {
      glDeleteVertexArrays(1, &mesh->VAO);
      glDeleteBuffers(1, &mesh->VBO);
      glDeleteBuffers(1, &mesh->EBO);
    }
  }
  void render(glm::mat4 global_model_matrix, float delta_time = 0.016f);
  std::vector<std::shared_ptr<Mesh>> meshes;

  Assimp::Importer import;
  const aiScene *scene;  

  bool hidden = false;

  float metallic = 0.1f;
  float roughness = 0.1f;

  std::vector<BoneInfo> boneInfos;
  std::unordered_map<std::string, unsigned int> boneMapping; // maps a bone name to its index
  std::vector<SkeletonBone> skeletonBones;

private:
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Shader> shader;
  std::string directory;

  void loadModel(const std::string &path);
  void processNode(aiNode *node, const aiScene *scene, int parentBoneIndex = -1);
  std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, glm::vec3 diffuseColor, glm::vec3 specularColor);
};
