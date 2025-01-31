#version 100
attribute vec4 vertex; // <vec2 pos, vec2 tex>
varying vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
