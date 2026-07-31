#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    TexCoord = aTexCoord;
    // Transform normal to world space (assuming no non-uniform scaling)
    Normal = mat3(model) * aNormal;
    Normal = normalize(Normal);
    // Compute fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
