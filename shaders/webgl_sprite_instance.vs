#version 100
attribute vec3 aPos;
attribute vec3 aColor;
attribute vec2 aTexCoord;

attribute mat4 aModel;
attribute float aIndex;
attribute vec2 aSpriteSize;

uniform mat4 projection;
uniform mat4 view;

uniform float time;
uniform float jitter;
uniform float jitter_speed;

varying vec3 ourColor;
varying vec2 TexCoord;
varying float index;
varying vec2 spriteSize;

float rand(vec2 co){
    return fract(sin( dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    float rand_x = (rand(aTexCoord * floor(time * jitter_speed)) * jitter);
    float rand_y = (rand(aTexCoord * floor(time * jitter_speed)) * jitter);
    gl_Position = projection * view * aModel * vec4(vec3(aPos.x + rand_x, aPos.y + rand_y, aPos.z), 1.0);
    ourColor = aColor;
    index = aIndex;
    spriteSize = aSpriteSize;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
