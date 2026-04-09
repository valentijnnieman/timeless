#version 300 es
in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aBoneIDs;
in vec4 aBoneWeights;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 boneMatrices[32];

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    float totalWeight = aBoneWeights.x + aBoneWeights.y + aBoneWeights.z + aBoneWeights.w;

    vec4 pos;
    vec3 norm;

    if (totalWeight > 0.001) {
        mat4 skinMatrix = aBoneWeights.x * boneMatrices[int(aBoneIDs.x)]
                        + aBoneWeights.y * boneMatrices[int(aBoneIDs.y)]
                        + aBoneWeights.z * boneMatrices[int(aBoneIDs.z)]
                        + aBoneWeights.w * boneMatrices[int(aBoneIDs.w)];
        pos  = skinMatrix * vec4(aPos, 1.0);
        norm = mat3(skinMatrix) * aNormal;
    } else {
        pos  = vec4(aPos, 1.0);
        norm = aNormal;
    }

    TexCoord    = aTexCoord;
    Normal      = normalize(mat3(model) * norm);
    FragPos     = vec3(model * pos);
    gl_Position = projection * view * model * pos;
}
