#include "timeless/components/model.hpp"
#include "glm/gtc/type_ptr.hpp"

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

    // then parse the materials
    aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

    // Diffuse color
    aiColor3D diffuseColor(0.f, 0.f, 0.f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

    // Specular color
    aiColor3D specularColor(0.f, 0.f, 0.f);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

    // Texture path
    aiString texPath;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
        // texPath.C_Str() gives you the texture filename
    }
    glm::vec3 diffuseColorVec = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    glm::vec3 specularColorVec = glm::vec3(specularColor.r, specularColor.g, specularColor.b);
    meshes.push_back(processMesh(aimesh, scene, diffuseColorVec, specularColorVec));
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }

}

std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, glm::vec3 diffuseColor, glm::vec3 specularColor) {
  std::vector<Vertex> vertices(mesh->mNumVertices);
  std::vector<unsigned int> indices;

  // Initialize vertex positions and texcoords
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.Position = glm::vec3(mesh->mVertices[i].x, -mesh->mVertices[i].y, mesh->mVertices[i].z);
    if (mesh->mTextureCoords[0]) {
      vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    } else {
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }
    vertex.Normal = glm::vec3(0.0f); // Will accumulate normals
    vertices[i] = vertex;
  }

  // Accumulate normals for smooth shading
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    // Get indices of the face
    unsigned int idx0 = face.mIndices[0];
    unsigned int idx1 = face.mIndices[1];
    unsigned int idx2 = face.mIndices[2];

    glm::vec3 v0 = vertices[idx0].Position;
    glm::vec3 v1 = vertices[idx1].Position;
    glm::vec3 v2 = vertices[idx2].Position;

    // Calculate face normal
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    // Accumulate face normal to each vertex normal
    vertices[idx0].Normal += faceNormal;
    vertices[idx1].Normal += faceNormal;
    vertices[idx2].Normal += faceNormal;

    // Add indices
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }


  }

  // Normalize accumulated normals
  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i].Normal = glm::normalize(vertices[i].Normal);
  }

  return std::make_shared<Mesh>(Mesh(vertices, indices, this->shader, diffuseColor, specularColor));
}

void Model::render() {
  for (unsigned int i = 0; i < meshes.size(); i++) {
    glBindVertexArray(meshes[i]->VAO);
    if(texture != nullptr) {
      texture->render();
    }
    glUniform3fv(glGetUniformLocation(shader->ID, "materialDiffuse"), 1,
                glm::value_ptr(meshes[i]->diffuseColor));
    glUniform3fv(glGetUniformLocation(shader->ID, "materialSpecular"), 1,
                glm::value_ptr(meshes[i]->specularColor));
    glUniform3fv(glGetUniformLocation(shader->ID, "lightColor"), 1,
                glm::value_ptr(glm::vec3(1.0f)));
    glUniform1i(glGetUniformLocation(shader->ID, "texture1"), 0);
    glDrawElements(GL_TRIANGLES,
                   static_cast<unsigned int>(meshes[i]->indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}
