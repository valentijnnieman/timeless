#version 300 es

in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;

// Per-instance attributes
in vec4 aModelMatrix0;
in vec4 aModelMatrix1;
in vec4 aModelMatrix2;
in vec4 aModelMatrix3;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
// World position in camera axes: the 1-bit dither pattern can be laid out
// on this (see world_dither in webgl_model_sun.fs).
out vec2 DitherPos;
// The screen slope of the face's horizontal edges (0 on floors and roofs),
// so the dither rows can run along walls (see world_dither in webgl_model_sun.fs).
out float DitherShear;

void main()
{
    mat4 aModelMatrix = mat4(aModelMatrix0, aModelMatrix1, aModelMatrix2, aModelMatrix3);
    FragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
    Normal = mat3(aModelMatrix) * aNormal;
    TexCoord = aTexCoords;
    DitherPos = (mat3(view) * FragPos).xy;
    // A wall's horizontal direction, and where the camera puts it on screen.
    vec3 wallDir = vec3(-Normal.y, Normal.x, 0.0);
    vec2 wallScreen = (mat3(view) * wallDir).xy;
    DitherShear = abs(normalize(Normal).z) < 0.5 && abs(wallScreen.x) > 0.05
                      ? clamp(wallScreen.y / wallScreen.x, -2.0, 2.0)
                      : 0.0;
    gl_Position = projection * view * aModelMatrix * vec4(aPos, 1.0);
}
