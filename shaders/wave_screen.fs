#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

float noiseIntensity = 0.0005;

vec2 wave(vec2 co)
{
    return vec2(sin(140 * PI * co.x), sin(140 * PI * co.y));
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

vec2 rand2(vec2 co)
{
    return vec2(rand(co), rand(co));
}

void main()
{ 
    float t = time / 20.0;
    vec2 noise = rand2(TexCoords + t) * noiseIntensity;
    vec2 wavey = wave(TexCoords + t) * noiseIntensity;
    FragColor = texture(screenTexture, vec2(TexCoords.x + wavey.x + noise.x, TexCoords.y + wavey.y + noise.y));
}