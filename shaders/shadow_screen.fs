#version 330 core
#define PI 3.1415926538
#define SAMPLES 2
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

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
    vec2 coords = TexCoords;

    vec4 col = texture(screenTexture, coords);

    FragColor = col * vec4(0.9, 0.9, 0.9, 0.5);
}  
