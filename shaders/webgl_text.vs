#version 100
attribute vec4 vertex; // <vec2 pos, vec2 tex>
varying vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

uniform float time;

float rand(vec2 co){
    return fract(sin( dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float str = 0.0;
float speed = 1.0;

void main()
{
    float rand_x = (rand(vertex.xy * floor(time * speed)) * str);
    float rand_y = (rand(vertex.xy * floor(time * speed)) * str);
    gl_Position = projection * view * model * vec4(vertex.x + rand_x, vertex.y + rand_y, 0.0, 1.0);
    TexCoords = vertex.zw;
}
