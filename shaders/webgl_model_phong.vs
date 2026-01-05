#version 100
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

void main()
{
    TexCoord = aTexCoord;
    // Transform normal to world space (assuming no non-uniform scaling)
    Normal = mat3(model) * aNormal;
    // Compute fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
