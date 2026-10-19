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
// When set (> 0), solid pixels take this as their alpha, the marker the 1-bit
// pass (webgl_screen_1bit.fs) treats as a citizen: items on the floor are
// then drawn undithered like them. The game sets it only while drawing the
// citizens' pass, with blending off, so see-through pixels are dropped rather
// than written over the scene.
uniform float markerAlpha;

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

    vec4 color = highlightColor * texture2D(texture1, uv);
    if (markerAlpha > 0.0) {
        if (color.a < 0.5)
            discard;
        color.a = markerAlpha;
    }
    gl_FragColor = color;

}
