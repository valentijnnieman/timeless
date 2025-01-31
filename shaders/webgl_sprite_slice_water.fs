#version 100
precision mediump float;
#define PI 3.1415926538
  
varying vec3 ourColor;
varying vec2 FragPos;
varying vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 highlightColor;   // In px
uniform vec2 spriteSheetSize;   // In px
uniform vec2 spriteSize;        // In px
uniform float index;            // Sprite index in sprite sheet (0-...)
uniform float col;
uniform float row;
uniform float time;

float noiseIntensity = 0.0005;

vec2 wave(vec2 co)
{
    return vec2(sin(140.0 * PI * co.x), sin(140.0 * PI * co.y));
}

void main()
{
    float w = spriteSheetSize.x;
    float h = spriteSheetSize.y;
    // Normalize sprite size (0.0-1.0)
    float dx = spriteSize.x / w;
    float dy = spriteSize.y / h;

    float t = time / 60.0;
    vec2 wavey = wave(TexCoord + t) * noiseIntensity;

    // Finally to UV texture coordinates
    vec2 uv = vec2(dx * TexCoord.x + col * dx, 1.0 - dy - row * dy + dy * TexCoord.y);

    gl_FragColor = highlightColor * texture2D(texture1, vec2(uv.x + wavey.x, uv.y + wavey.y));
}
