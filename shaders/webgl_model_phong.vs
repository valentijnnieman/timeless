#version 100
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
// Skinning attributes (match Mesh::setupMesh and webgl_model_cooktor.vs). They
// are optional: meshes without a skeleton leave these zero and useSkinning off.
attribute vec4 aBoneWeights;
attribute vec4 aBoneIDs;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

// Skeletal animation: poseMatrices uploaded by RenderingSystem::upload_bone_matrices.
uniform mat4 boneMatrices[128];
uniform bool useSkinning;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

void main()
{
    TexCoord = aTexCoord;

    vec4 skinnedPos;
    vec3 skinnedNormal;

    float wsum = aBoneWeights.x + aBoneWeights.y + aBoneWeights.z + aBoneWeights.w;
    if (useSkinning && wsum > 0.0001) {
        mat4 skinMatrix =
            aBoneWeights.x * boneMatrices[int(aBoneIDs.x)] +
            aBoneWeights.y * boneMatrices[int(aBoneIDs.y)] +
            aBoneWeights.z * boneMatrices[int(aBoneIDs.z)] +
            aBoneWeights.w * boneMatrices[int(aBoneIDs.w)];
        skinnedPos = skinMatrix * vec4(aPos, 1.0);
        skinnedNormal = mat3(skinMatrix) * aNormal;
    } else {
        // No skinning, or an unweighted vertex: keep it at its bind position
        // instead of collapsing to the origin (skinMatrix would be the 0 matrix).
        skinnedPos = vec4(aPos, 1.0);
        skinnedNormal = aNormal;
    }

    // Transform normal to world space (assuming no non-uniform scaling)
    Normal = mat3(model) * skinnedNormal;
    // Compute fragment position in world space
    FragPos = vec3(model * skinnedPos);
    gl_Position = projection * view * model * skinnedPos;
}
