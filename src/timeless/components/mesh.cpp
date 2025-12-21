#include "timeless/components/mesh.hpp"
#include <memory>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Shader> shader) {
  this->vertices = vertices;
  this->indices = indices;

  setupMesh(shader);
}

void Mesh::setupMesh(std::shared_ptr<Shader> shader) {
  for(int i = 0; i < indices.size(); i++) {
    std::cout << indices[i] << " ";
  }
  std::cout << std::endl;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("generating buffers caused an OpenGL error: %x\n", err);
  }

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("glBufferData caused an OpenGL error: %x\n", err);
  }

  GLint posLoc = glGetAttribLocation(shader->ID, "aPos");
  GLint normLoc = glGetAttribLocation(shader->ID, "aNormal");
  GLint texLoc = glGetAttribLocation(shader->ID, "aTexCoord");
  // std::cout << "Model Attribute Locations - Pos: " << posLoc
  //           << ", Norm: " << normLoc
  //           << ", TexCoord: " << texLoc << std::endl;
  // vertex positions
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  // vertex normals
  glEnableVertexAttribArray(normLoc);
  glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));
  // vertex texture coords
  glEnableVertexAttribArray(texLoc);
  glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, TexCoords));

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("OpenGL error: %x\n", err);
  }

  glBindVertexArray(0);
}
