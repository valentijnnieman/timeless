#include "timeless/components/model.hpp"
#include "glm/gtc/type_ptr.hpp"

void Model::loadModel(const std::string &path) {
  scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);  
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));

  processNode(scene->mRootNode, scene);
  // Fill childrenIndices for each bone
  std::map<std::string, int> nameToIndex;
  for (int i = 0; i < skeletonBones.size(); ++i) {
      nameToIndex[skeletonBones[i].name] = i;
  }
  for (int i = 0; i < skeletonBones.size(); ++i) {
      int parentIdx = skeletonBones[i].parentIndex;
      if (parentIdx != -1 && parentIdx < skeletonBones.size()) {
          skeletonBones[parentIdx].childrenIndices.push_back(i);
      }
  }
}

void Model::processNode(aiNode *node, const aiScene *scene, int parentBoneIndex) {
    // If this node is a bone, record it in the skeleton hierarchy
    auto it = boneMapping.find(node->mName.C_Str());
    int currentBoneIndex = -1;
    if (it != boneMapping.end()) {
        currentBoneIndex = it->second;
        SkeletonBone bone;
        bone.name = node->mName.C_Str();
        bone.parentIndex = parentBoneIndex;
        // Children will be filled in below
        skeletonBones.push_back(bone);
        // Optionally: store index mapping for quick lookup
    }
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
        int childBoneIndex = -1;
        // Recursively process child, passing currentBoneIndex as parent
        processNode(node->mChildren[i], scene, currentBoneIndex);
  }

}

std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, glm::vec3 diffuseColor, glm::vec3 specularColor) {
  std::vector<Vertex> vertices(mesh->mNumVertices);
  std::vector<unsigned int> indices;


  // Initialize vertex positions and texcoords
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
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

  //process bones
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
      aiBone* bone = mesh->mBones[i];
      std::string boneName(bone->mName.data);

      int boneIndex = 0;
      auto it = boneMapping.find(boneName);
      if (it == boneMapping.end()) {
          boneIndex = boneInfos.size();
          boneMapping[boneName] = boneIndex;

          BoneInfo boneInfo;
          // Convert aiMatrix4x4 to glm::mat4
          aiMatrix4x4 offset = bone->mOffsetMatrix;
          boneInfo.offsetMatrix = glm::mat4(
              offset.a1, offset.b1, offset.c1, offset.d1,
              offset.a2, offset.b2, offset.c2, offset.d2,
              offset.a3, offset.b3, offset.c3, offset.d3,
              offset.a4, offset.b4, offset.c4, offset.d4
          );
          boneInfos.push_back(boneInfo);
      } else {
          boneIndex = it->second;
      }

      for (unsigned int j = 0; j < bone->mNumWeights; j++) {
          unsigned int vertexID = bone->mWeights[j].mVertexId;
          float weight = bone->mWeights[j].mWeight;
          for (int i = 0; i < 4; ++i) {
              if (vertices[vertexID].boneData.weights[i] == 0.0f) {
                  vertices[vertexID].boneData.ids[i] = static_cast<float>(boneIndex);
                  vertices[vertexID].boneData.weights[i] = weight;
              }
          }
      }
  }

  return std::make_shared<Mesh>(Mesh(vertices, indices, boneInfos, boneMapping, this->shader, diffuseColor, specularColor));
}

void Model::render(glm::mat4 global_model_matrix, float delta_time) {
  if(!hidden) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
      glBindVertexArray(meshes[i]->VAO);
      // Set the model matrix uniform
      meshes[i]->update_animation(delta_time);
      glm::mat4 meshModelMatrix = global_model_matrix * meshes[i]->getModelMatrix();
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(meshModelMatrix));

      glUniform3fv(glGetUniformLocation(shader->ID, "materialDiffuse"), 1,
                  glm::value_ptr(meshes[i]->diffuseColor));
      glUniform3fv(glGetUniformLocation(shader->ID, "albedo"), 1,
                  glm::value_ptr(meshes[i]->diffuseColor));
      glUniform1f(glGetUniformLocation(shader->ID, "metallic"), metallic);
      glUniform1f(glGetUniformLocation(shader->ID, "roughness"), roughness);
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
}
