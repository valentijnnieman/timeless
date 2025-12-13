#version 100
attribute vec3 aPos;
attribute vec3 aColor;
attribute vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

varying vec3 ourColor;
varying vec2 TexCoord;
varying vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(vec3(aPos.x, aPos.y, aPos.z), 1.0);
    ourColor = aColor;
    TexCoord = vec2(aTexCoord.x , aTexCoord.y);
    FragPos = (projection * view * model * vec4(aPos, 1.0)).xyz; // World position
}
