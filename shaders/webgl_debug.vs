#version 100
attribute vec3 aPos;
attribute vec4 aColor;

uniform mat4 projection;
uniform mat4 view;

varying vec4 fragColor;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    fragColor = aColor;
}
