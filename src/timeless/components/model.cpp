#include "timeless/components/model.hpp"
#include <memory>

void Model::loadModel(const std::string &path) {
  Assimp::Importer import;
  const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);  
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));

  processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *aimesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(aimesh, scene));
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      unsigned int index = face.mIndices[j];
      Vertex vertex;
      // Position
      vertex.Position =
          glm::vec3(mesh->mVertices[index].x, mesh->mVertices[index].y,
                    mesh->mVertices[index].z);
      // Normal
      if (mesh->HasNormals()) {
        vertex.Normal =
            glm::vec3(mesh->mNormals[index].x, mesh->mNormals[index].y,
                      mesh->mNormals[index].z);
      }
      // TexCoords
      if (mesh->mTextureCoords[0]) {
        vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][index].x,
                                     mesh->mTextureCoords[0][index].y);
      } else {
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
      }
      vertices.push_back(vertex);
      indices.push_back(vertices.size() - 1); // sequential index
      //
    }
  }
  return std::make_shared<Mesh>(Mesh(vertices, indices, this->shader));
}

void Model::render() {
  for (unsigned int i = 0; i < meshes.size(); i++) {
    glBindVertexArray(meshes[i]->VAO);
    texture->render();
    glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
    glDrawElements(GL_TRIANGLES,
                   static_cast<unsigned int>(meshes[i]->indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}
