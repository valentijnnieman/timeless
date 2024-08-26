#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec2 FragPos;

uniform sampler2D texture1;
uniform vec4 highlightColor;   // In px
uniform vec2 spriteSheetSize;   // In px
uniform vec2 spriteSize;        // In px
uniform float index;            // Sprite index in sprite sheet (0-...)
uniform float time;

float noiseIntensity = 0.0005;

vec2 wave(vec2 co)
{
    return vec2(sin(140 * PI * co.x), sin(140 * PI * co.y));
}

void main()
{
    float w = spriteSheetSize.x;
    float h = spriteSheetSize.y;
    // Normalize sprite size (0.0-1.0)
    float dx = spriteSize.x / w;
    float dy = spriteSize.y / h;
    // Figure out number of tile cols of sprite sheet
    float cols = w / spriteSize.x;
    // From linear index to row/col pair
    float col = mod(index, cols);
    float row = floor(index / cols);

    float t = time / 60.0;
    vec2 wavey = wave(TexCoord + t) * noiseIntensity;

    // Finally to UV texture coordinates
    vec2 uv = vec2(dx * TexCoord.x + col * dx, 1.0 - dy - row * dy + dy * TexCoord.y);

    FragColor = highlightColor * texture(texture1, vec2(uv.x + wavey.x, uv.y + wavey.y));
    // float step = 1.0 / frameWidth;
    // vec2 pos = vec2((step * (xIndex * 1.0)) + (TexCoord.x / xAmount), (step * (yIndex * 1.0)) + (TexCoord.y / yAmount));
    // // pos = fract(pos + vec2(step * 1.0, 0.0));
    // FragColor = texture(texture1, pos);
}