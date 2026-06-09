#version 100
attribute vec3 aPos;       // quad local, -1..1 in XY (lies flat in the XY plane)
attribute vec3 aColor;     // unused
attribute vec2 aTexCoord;  // 0..1

uniform mat4 projection;
uniform mat4 model;        // from the entity's Transform
uniform mat4 view;

varying vec2 vUV;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vUV = aTexCoord;
}
