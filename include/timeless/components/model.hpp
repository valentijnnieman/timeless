#pragma once
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "timeless/components/component.hpp"
#include "timeless/components/mesh.hpp"
#include "timeless/components/texture.hpp"
#include <memory>

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
  void render(glm::mat4 global_model_matrix, float delta_time = 0.016f);
  std::vector<std::shared_ptr<Mesh>> meshes;

  bool hidden = false;

  float metallic = 0.1f;
  float roughness = 0.1f;
private:
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Shader> shader;
  std::string directory;

  void loadModel(const std::string &path);
  void processNode(aiNode *node, const aiScene *scene);
  std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, glm::vec3 diffuseColor, glm::vec3 specularColor);
};
