#include "timeless/components/model.hpp"
#include "assimp/config.h"
#include "assimp/postprocess.h"
#include "glm/gtc/type_ptr.hpp"

static glm::mat4 aiToGlm(const aiMatrix4x4 &m) {
  // Assimp is row-major; GLM is column-major — transpose on construction.
  return glm::mat4(
    m.a1, m.b1, m.c1, m.d1,
    m.a2, m.b2, m.c2, m.d2,
    m.a3, m.b3, m.c3, m.d3,
    m.a4, m.b4, m.c4, m.d4
  );
}

void Model::collectBones(const aiScene* scene) {
  // Traverse all meshes and collect bones into boneMapping and boneInfos
  for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
    aiMesh* mesh = scene->mMeshes[meshIdx];
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
      aiBone* bone = mesh->mBones[i];
      std::string boneName(bone->mName.data);
      if (boneMapping.find(boneName) == boneMapping.end()) {
        int boneIndex = boneInfos.size();
        boneMapping[boneName] = boneIndex;

        BoneInfo boneInfo;
        boneInfo.offsetMatrix = aiToGlm(bone->mOffsetMatrix);
        boneInfos.push_back(boneInfo);
      }
    }
  }
}

void Model::loadModel(const std::string &path, bool from_blender) {
  // Scale the entire scene by 0.01 to convert from FBX centimetres (Blender's
  // default FBX export unit) to metres. Assimp applies this uniformly to
  // vertex positions, node transforms, and bone offset matrices so the
  // skinning math stays consistent.
  import.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);

  unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GlobalScale;
  if (from_blender) flags |= aiProcess_ConvertToLeftHanded;
  scene = import.ReadFile(path, flags);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));

  // First, collect all bones for the model
  collectBones(scene);

  processNode(scene->mRootNode, scene, glm::mat4(1.0f));
  processBone(scene->mRootNode, scene, -1, glm::mat4(1.0f));

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

void Model::processBone(aiNode *node, const aiScene *scene, int parentSkeletonIdx, glm::mat4 parentTransform) {
  glm::mat4 globalTransform = parentTransform * aiToGlm(node->mTransformation);

  int currentSkeletonIdx = -1;
  auto it = boneMapping.find(node->mName.C_Str());
  if (it != boneMapping.end()) {
    // This is a bone node. If it's a root bone (no skeleton parent yet), record
    // the accumulated transform of all non-bone ancestor nodes so the animation
    // system can use it as the initial parent transform.
    if (parentSkeletonIdx == -1) {
      skeletonRootNodeTransform = parentTransform;
    }
    currentSkeletonIdx = (int)skeletonBones.size();
    SkeletonBone bone;
    bone.name = node->mName.C_Str();
    bone.parentIndex = parentSkeletonIdx;
    skeletonBones.push_back(bone);
  }

  // Propagate the last known skeleton index so bones under non-bone nodes
  // still get the correct parent (fixes the case where a non-bone scene node
  // sits between two bone nodes in the hierarchy).
  int nextParentIdx = (currentSkeletonIdx != -1) ? currentSkeletonIdx : parentSkeletonIdx;

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processBone(node->mChildren[i], scene, nextParentIdx, globalTransform);
  }
}

void Model::processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform) {
  glm::mat4 globalTransform = parentTransform * aiToGlm(node->mTransformation);

  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *aimesh = scene->mMeshes[node->mMeshes[i]];

    aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

    aiColor3D diffuseColor(0.f, 0.f, 0.f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

    aiColor3D specularColor(0.f, 0.f, 0.f);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

    aiString texPath;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
    }
    glm::vec3 diffuseColorVec  = glm::vec3(diffuseColor.r,  diffuseColor.g,  diffuseColor.b);
    glm::vec3 specularColorVec = glm::vec3(specularColor.r, specularColor.g, specularColor.b);
    size_t meshIndex = meshes.size();
    auto mesh = processMesh(aimesh, scene, globalTransform, diffuseColorVec, specularColorVec);
    mesh->nodeName = node->mName.C_Str();
    mesh->nodeParentTransform = parentTransform;
    meshNodeMap[node->mName.C_Str()] = meshIndex;
    meshes.push_back(mesh);
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, globalTransform);
  }
}

std::shared_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 nodeTransform, glm::vec3 diffuseColor, glm::vec3 specularColor) {
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
    // Initialize bone data to zero
    for (int j = 0; j < 4; ++j) {
      vertex.boneData.ids[j] = 0.0f;
      vertex.boneData.weights[j] = 0.0f;
    }
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

  // Assign bone indices and weights to vertices using the global boneMapping
  for (unsigned int i = 0; i < mesh->mNumBones; i++) {
      aiBone* bone = mesh->mBones[i];
      std::string boneName(bone->mName.data);

      auto it = boneMapping.find(boneName);
      if (it == boneMapping.end()) {
          // Should not happen, boneMapping should be complete
          continue;
      }
      int boneIndex = it->second;

      for (unsigned int j = 0; j < bone->mNumWeights; j++) {
          unsigned int vertexID = bone->mWeights[j].mVertexId;
          float weight = bone->mWeights[j].mWeight;
          for (int k = 0; k < 4; ++k) {
              if (vertices[vertexID].boneData.weights[k] == 0.0f) {
                  vertices[vertexID].boneData.ids[k] = static_cast<float>(boneIndex);
                  vertices[vertexID].boneData.weights[k] = weight;
                  break;
              }
          }
      }
  }

  std::shared_ptr<Mesh> result;
  if (mesh->mNumBones > 0) {
    result = std::make_shared<Mesh>(Mesh(vertices, indices, boneInfos, boneMapping, this->shader, diffuseColor, specularColor));
  } else {
    result = std::make_shared<Mesh>(Mesh(vertices, indices, this->shader, diffuseColor, specularColor));
  }
  result->nodeTransform = nodeTransform;
  return result;
}

void Model::render(glm::mat4 global_model_matrix, float delta_time, bool use_skinning) {
  if(!hidden) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
      glBindVertexArray(meshes[i]->VAO);
      // Set the model matrix uniform
      meshes[i]->update_animation(delta_time);
      glm::mat4 meshModelMatrix = global_model_matrix * meshes[i]->getModelMatrix();
      glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(meshModelMatrix));

      GLint useSkinningLoc = glGetUniformLocation(shader->ID, "useSkinning");

      // if(meshes[i]->boneInfos.size() > 0) {
      //   glUniform1i(useSkinningLoc, 1);
      // }
      // else {
        glUniform1i(useSkinningLoc, 0);
      // }

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
void Model::instanced_render(GLsizei instance_count) {
  for (unsigned int i = 0; i < meshes.size(); i++) {
    glBindVertexArray(meshes[i]->VAO);
    glDrawElementsInstanced(GL_TRIANGLES,
                            static_cast<unsigned int>(meshes[i]->indices.size()),
                            GL_UNSIGNED_INT, 0, instance_count);
    glBindVertexArray(0);
  }
}
