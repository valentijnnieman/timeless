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
// World position in camera axes: the 1-bit dither pattern can be laid out
// on this (see world_dither in webgl_model_sun.fs).
out vec2 DitherPos;
// The screen slope of the face's horizontal edges (0 on floors and roofs),
// so the dither rows can run along walls (see world_dither in webgl_model_sun.fs).
out float DitherShear;

void main()
{
    TexCoord = aTexCoord;
    // Transform normal to world space (assuming no non-uniform scaling)
    Normal = mat3(model) * aNormal;
    Normal = normalize(Normal);
    // Compute fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    DitherPos = (mat3(view) * FragPos).xy;
    // A wall's horizontal direction, and where the camera puts it on screen.
    vec3 wallDir = vec3(-Normal.y, Normal.x, 0.0);
    vec2 wallScreen = (mat3(view) * wallDir).xy;
    DitherShear = abs(normalize(Normal).z) < 0.5 && abs(wallScreen.x) > 0.05
                      ? clamp(wallScreen.y / wallScreen.x, -2.0, 2.0)
                      : 0.0;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
