#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec2 FragPos;

uniform sampler2D texture1;
uniform vec4 highlightColor;   // In px
uniform vec2 spriteSheetSize;   // In px
uniform vec2 spriteSize;        // In px
uniform float index;            // Sprite index in sprite sheet (0-...)
uniform float tick;
uniform float time;
uniform vec2 lightPosition;
uniform vec3 cameraPosition;

vec2 u_resolution = vec2(1920, 1080);

vec2 lp = vec2(0.1 * tick, 0.1 * tick);


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
    // Finally to UV texture coordinates
    vec2 uv = vec2(dx * TexCoord.x + col * dx, 1.0 - dy - row * dy + dy * TexCoord.y);

    vec2 st =  (cameraPosition.xy / u_resolution);
    float f = 0.2 + step(0.66, dot(st, lp));
    float a = 0.0 + step(0.33, dot(TexCoord, lp));

    vec4 c = vec4(clamp(f + a, 0.0, 1.0));
    c.a = 1.0;

    FragColor = c * highlightColor * texture(texture1, uv);
    // float step = 1.0 / frameWidth;
    // vec2 pos = vec2((step * (xIndex * 1.0)) + (TexCoord.x / xAmount), (step * (yIndex * 1.0)) + (TexCoord.y / yAmount));
    // // pos = fract(pos + vec2(step * 1.0, 0.0));
    // FragColor = texture(texture1, pos);
}