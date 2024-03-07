#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

void main()
{
    FragColor = vec4(vec3(texture(screenTexture, TexCoords)), 1.0);
}  