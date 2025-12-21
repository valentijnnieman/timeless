#version 100
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

varying vec2 TexCoord;
varying vec3 Normal;

void main()
{
    TexCoord = aTexCoord;
    Normal = aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
