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
  void render();
  std::vector<std::shared_ptr<Mesh>> meshes;

private:
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Shader> shader;
  std::string directory;

  void loadModel(const std::string &path);
  void processNode(aiNode *node, const aiScene *scene);
  std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene);
};
