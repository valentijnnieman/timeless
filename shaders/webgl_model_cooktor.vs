#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aBoneWeights;
layout(location = 4) in vec4 aBoneIDs;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 boneMatrices[128];
uniform bool useSkinning;
uniform mat4 lightSpaceMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

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

    Normal = normalize(mat3(model) * skinnedNormal);
    FragPos = vec3(model * skinnedPos);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * model * skinnedPos;
}
