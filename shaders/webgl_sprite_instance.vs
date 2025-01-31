#version 100
attribute vec3 aPos;
attribute vec3 aColor;
attribute vec2 aTexCoord;

attribute mat4 aModel;
attribute float aIndex;
attribute vec2 aSpriteSize;

uniform mat4 projection;
uniform mat4 view;

varying vec3 ourColor;
varying vec2 TexCoord;
varying float index;
varying vec2 spriteSize;

void main()
{
    gl_Position = projection * view * aModel * vec4(aPos, 1.0);
    ourColor = aColor;
    index = aIndex;
    spriteSize = aSpriteSize;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
