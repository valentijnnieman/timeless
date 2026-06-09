#version 100
attribute vec3 aPos;       // quad local, -1..1 in XY (lies flat in the XY plane)
attribute vec3 aColor;     // unused
attribute vec2 aTexCoord;  // unused

uniform mat4 projection;
uniform mat4 model;        // from the entity's Transform (UI space)
uniform mat4 view;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
