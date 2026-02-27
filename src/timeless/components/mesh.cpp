#include "timeless/components/mesh.hpp"
#include <memory>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::shared_ptr<Shader> shader, glm::vec3 diffuseColor,
           glm::vec3 specularColor)
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f),
      scale(1.0f, 1.0f, 1.0f) {
  this->vertices = vertices;
  this->indices = indices;
  this->diffuseColor = diffuseColor;
  this->specularColor = specularColor;

  setupMesh(shader);
}
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
           std::vector<BoneInfo> boneInfos,
           std::unordered_map<std::string, unsigned int> boneMapping,
           std::shared_ptr<Shader> shader, glm::vec3 diffuseColor,
           glm::vec3 specularColor)
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f),
      scale(1.0f, 1.0f, 1.0f) {
  this->vertices = vertices;
  this->indices = indices;
  this->diffuseColor = diffuseColor;
  this->specularColor = specularColor;
  this->boneInfos = boneInfos;
  this->boneMapping = boneMapping;

  setupMesh(shader);
}

void Mesh::setupMesh(std::shared_ptr<Shader> shader) {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // GLenum err = glGetError();
  // if (err != GL_NO_ERROR) {
  //   printf("generating buffers caused an OpenGL error: %x\n", err);
  // }

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // err = glGetError();
  // if (err != GL_NO_ERROR) {
  //   printf("glBufferData caused an OpenGL error: %x\n", err);
  // }

  GLint posLoc = glGetAttribLocation(shader->ID, "aPos");
  GLint normLoc = glGetAttribLocation(shader->ID, "aNormal");
  GLint texLoc = glGetAttribLocation(shader->ID, "aTexCoord");
  GLint boneIDsLoc = glGetAttribLocation(shader->ID, "aBoneIDs");
  GLint boneWeightsLoc = glGetAttribLocation(shader->ID, "aBoneWeights");
  // std::cout << "Model Attribute Locations - Pos: " << posLoc
  //           << ", Norm: " << normLoc
  //           << ", TexCoord: " << texLoc << std::endl;
  // vertex positions
  if (posLoc >= 0) {
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)0);
  }
  // vertex normals
  if (normLoc >= 0) {
    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));
  }
  // vertex texture coords
  if (texLoc >= 0) {
    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));
  }

  if(boneInfos.size() > 0) {
    if (boneIDsLoc >= 0) {
        glEnableVertexAttribArray(boneIDsLoc);
        glVertexAttribPointer(boneIDsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneData.ids));
    }

    if (boneWeightsLoc >= 0) {
        glEnableVertexAttribArray(boneWeightsLoc);
        glVertexAttribPointer(boneWeightsLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneData.weights));
    }
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
