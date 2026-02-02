#version 100
precision mediump float;

uniform sampler2D screenTexture;
varying vec2 TexCoords;

// 4x4 Bayer matrix function
float bayer(vec2 pos) {
    float x = mod(pos.x, 4.0);
    float y = mod(pos.y, 4.0);
    float threshold = 0.0;
    if (x == 0.0 && y == 0.0) threshold = 0.0;
    else if (x == 1.0 && y == 0.0) threshold = 8.0;
    else if (x == 2.0 && y == 0.0) threshold = 2.0;
    else if (x == 3.0 && y == 0.0) threshold = 10.0;
    else if (x == 0.0 && y == 1.0) threshold = 12.0;
    else if (x == 1.0 && y == 1.0) threshold = 4.0;
    else if (x == 2.0 && y == 1.0) threshold = 14.0;
    else if (x == 3.0 && y == 1.0) threshold = 6.0;
    else if (x == 0.0 && y == 2.0) threshold = 3.0;
    else if (x == 1.0 && y == 2.0) threshold = 11.0;
    else if (x == 2.0 && y == 2.0) threshold = 1.0;
    else if (x == 3.0 && y == 2.0) threshold = 9.0;
    else if (x == 0.0 && y == 3.0) threshold = 15.0;
    else if (x == 1.0 && y == 3.0) threshold = 7.0;
    else if (x == 2.0 && y == 3.0) threshold = 13.0;
    else if (x == 3.0 && y == 3.0) threshold = 5.0;
    return threshold / 16.0;
}

void main() {
    vec2 pixelPos = gl_FragCoord.xy;
    vec4 color = texture2D(screenTexture, TexCoords);

    float levels = 4.0;
    float threshold = bayer(pixelPos);

    vec3 dithered = floor(color.rgb* (levels - 1.0) + threshold) / (levels - 1.0);

    gl_FragColor = vec4(noisyColor, color.a);
}
