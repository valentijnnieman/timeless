#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 3) in mat4 aModel;
layout (location = 7) in float aIndex;
layout (location = 8) in vec2 aSpriteSize;

uniform mat4 projection;
uniform mat4 view;

out vec3 ourColor;
out vec2 TexCoord;
out float index;
out vec2 spriteSize;

void main()
{
    gl_Position = projection * view * aModel * vec4(aPos, 1.0);
    ourColor = aColor;
    index = aIndex;
    spriteSize = aSpriteSize;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
