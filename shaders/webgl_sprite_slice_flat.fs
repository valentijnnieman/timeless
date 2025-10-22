#version 100
precision mediump float;

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

float mod2(float x, float y) {
  return x - y * floor(x/y);
}

void main()
{
    float w = spriteSheetSize.x;
    float h = spriteSheetSize.y;
    // Normalize sprite size (0.0-1.0)
    float dx = spriteSize.x / w;
    float dy = spriteSize.y / h;
    // Figure out number of tile cols of sprite sheet
    // From linear index to row/col pair
    // Finally to UV texture coordinates
    vec2 uv = vec2(dx * TexCoord.x + col * dx, 1.0 - dy - row * dy + dy * TexCoord.y);

    gl_FragColor = vec4(highlightColor.rgb, texture2D(texture1, uv).a * highlightColor.a);

}
