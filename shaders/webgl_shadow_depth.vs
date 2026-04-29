#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 3) in vec4 aBoneWeights;
layout(location = 4) in vec4 aBoneIDs;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform mat4 boneMatrices[32];
uniform bool useSkinning;

void main()
{
    vec4 pos;
    if (useSkinning) {
        mat4 skinMatrix =
            aBoneWeights.x * boneMatrices[int(aBoneIDs.x)] +
            aBoneWeights.y * boneMatrices[int(aBoneIDs.y)] +
            aBoneWeights.z * boneMatrices[int(aBoneIDs.z)] +
            aBoneWeights.w * boneMatrices[int(aBoneIDs.w)];
        pos = skinMatrix * vec4(aPos, 1.0);
    } else {
        pos = vec4(aPos, 1.0);
    }
    gl_Position = lightSpaceMatrix * model * pos;
}
