#version 100
attribute vec3 aPos;
attribute vec3 aColor;
attribute vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
