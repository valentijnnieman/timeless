#version 100
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

attribute vec4 aBoneWeights;
attribute vec4 aBoneIDs;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 boneMatrices[32];
uniform bool useSkinning;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

void main()
{
    TexCoord = aTexCoord;

    vec4 skinnedPos;
    vec3 skinnedNormal;

    if (useSkinning) {
        mat4 skinMatrix =
            aBoneWeights.x * boneMatrices[int(aBoneIDs.x)] +
            aBoneWeights.y * boneMatrices[int(aBoneIDs.y)] +
            aBoneWeights.z * boneMatrices[int(aBoneIDs.z)] +
            aBoneWeights.w * boneMatrices[int(aBoneIDs.w)];
        skinnedPos = skinMatrix * vec4(aPos, 1.0);
        skinnedNormal = mat3(skinMatrix) * aNormal;
    } else {
        skinnedPos = vec4(aPos, 1.0);
        skinnedNormal = aNormal;
    }

    Normal = normalize(skinnedNormal);
    FragPos = vec3(model * skinnedPos);
    gl_Position = projection * view * model * skinnedPos;
}
