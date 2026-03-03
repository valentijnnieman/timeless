#version 300 es

in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;

// Per-instance attributes
in vec4 aModelMatrix0;
in vec4 aModelMatrix1;
in vec4 aModelMatrix2;
in vec4 aModelMatrix3;
in float aModelIndex;
in vec4 aModelParams;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out float ModelIndex;
out vec4 ModelParams;

void main()
{
    mat4 aModelMatrix = mat4(aModelMatrix0, aModelMatrix1, aModelMatrix2, aModelMatrix3);
    FragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
    Normal = mat3(aModelMatrix) * aNormal;
    TexCoord = aTexCoords;
    ModelIndex = aModelIndex;
    ModelParams = aModelParams;

    gl_Position = projection * view * aModelMatrix * vec4(aPos, 1.0);
}
