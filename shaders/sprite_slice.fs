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
uniform float col;
uniform float row;

uniform float resolution_downsampling = 2.0;
uniform float bit_depth = 32.0;

const mat4 bayer_matrix = mat4(
    vec4(    -0.5,       0.0,  -0.375,   0.125 ),
    vec4(    0.25,   -0.25,   0.375, - 0.125 ),
    vec4( -0.3125,  0.1875, -0.4375,  0.0625 ),
    vec4(  0.4375, -0.0625,  0.3125, -0.1875 )
);


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

    vec4 color = highlightColor * texture(texture1, uv);
    vec3 tex = color.rgb;

    vec2 pix_id = vec2(uv.x / (2.0 * resolution_downsampling), uv.y / (2.0 * resolution_downsampling));
    float bayer_shift = bayer_matrix[int(mod(pix_id.x, 4.0))][int(mod(pix_id.y, 4.0))];
    tex += vec3(bayer_shift / bit_depth);
    tex.r = round(tex.r * bit_depth-1.0) / (bit_depth-1.0);
    tex.g = round(tex.g * bit_depth-1.0) / (bit_depth-1.0);
    tex.b = round(tex.b * bit_depth-1.0) / (bit_depth-1.0);
    color.rgb = tex.rgb;
    FragColor = color;

}
