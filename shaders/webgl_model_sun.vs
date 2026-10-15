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
// World position in camera axes, without the pan: the 1-bit dither pattern
// is laid out on this (see world_dither in webgl_model_sun.fs).
out vec2 DitherPos;
uniform vec3 ditherOffset;

void main()
{
    TexCoord = aTexCoord;
    // Transform normal to world space (assuming no non-uniform scaling)
    Normal = mat3(model) * aNormal;
    Normal = normalize(Normal);
    // Compute fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    DitherPos = (mat3(view) * (FragPos + ditherOffset)).xy;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
