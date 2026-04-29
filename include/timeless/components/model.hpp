#pragma once
#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "timeless/components/component.hpp"
#include "timeless/components/mesh.hpp"
#include "timeless/components/texture.hpp"
#include <memory>
#include <map>
#include <optional>
#include <unordered_map>

struct SkeletonBone {
    std::string name;
    int parentIndex; // -1 if root
    std::vector<int> childrenIndices;
};

class Model : public Component {
public:
  Assimp::Importer import;
  const aiScene *scene;  

  bool hidden = false;

  std::optional<float> metallic;
  std::optional<float> roughness;

  float index = 0.0f;
  glm::vec4 params = glm::vec4(0.1f, 0.1f, 0.0f, 0.0f); // Example: metallic, roughness, unused, unused

  Model(const std::string &path, std::shared_ptr<Texture> texture,
        std::shared_ptr<Shader> shader)
      : texture(texture), shader(shader) {
    loadModel(path);
  }
  Model(const std::string &path, std::shared_ptr<Shader> shader, bool from_blender = false)
      : shader(shader) {
    loadModel(path, from_blender);
  }
  Model(const std::string &path, std::shared_ptr<Shader> shader, float index, bool from_blender = false)
      : shader(shader), index(index) {
    loadModel(path, from_blender);
  }
  ~Model() {
    for (auto &mesh : meshes) {
      glDeleteVertexArrays(1, &mesh->VAO);
      glDeleteBuffers(1, &mesh->VBO);
      glDeleteBuffers(1, &mesh->EBO);
    }
  }
  void render(glm::mat4 global_model_matrix, float delta_time = 0.016f, bool use_skinning = false);
  void instanced_render(GLsizei instance_count);
  std::vector<std::shared_ptr<Mesh>> meshes;

  std::vector<BoneInfo> boneInfos;
  std::unordered_map<std::string, unsigned int> boneMapping; // maps a bone name to its index
  std::vector<SkeletonBone> skeletonBones;
  // Accumulated transform of all non-bone scene nodes above the skeleton root.
  // Used as the initial parent transform when computing animated bone poses.
  glm::mat4 skeletonRootNodeTransform = glm::mat4(1.0f);

  // Maps node name -> index into meshes[], for applying object-level animations
  // directly to mesh nodeTransforms (non-skinned / no-bone objects).
  std::unordered_map<std::string, size_t> meshNodeMap;

  // Optional normal map for webgl_model_cooktor_normal shader.
  // Bound to GL_TEXTURE2 during render().
  std::shared_ptr<Texture> normal_texture;

private:
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Shader> shader;
  std::string directory;

  void collectBones(const aiScene* scene);
  void loadModel(const std::string &path, bool from_blender = false);
  void processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform = glm::mat4(1.0f));
  void processBone(aiNode *node, const aiScene *scene, int parentBoneIndex = -1, glm::mat4 parentTransform = glm::mat4(1.0f));
  std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 nodeTransform, glm::vec3 diffuseColor, glm::vec3 specularColor);
};
