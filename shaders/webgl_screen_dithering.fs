#version 100
precision highp float;

uniform sampler2D screenTexture;
varying vec2 TexCoords;

// 4x4 Bayer matrix function
float bayer4(vec2 p) {
    p = floor(mod(p, 4.0));

    return (
        8.0 * mod(p.x, 2.0) +
        4.0 * mod(p.y, 2.0) +
        2.0 * mod(floor(p.x / 2.0), 2.0) +
        mod(floor(p.y / 2.0), 2.0)
    ) / 16.0;
}

void main() {
    vec2 pixelPos = gl_FragCoord.xy;
    vec4 color = texture2D(screenTexture, TexCoords);

    float levels = 4.0; // Number of quantization levels per channel
    float threshold = bayer4(pixelPos);

    // Dithered quantization
    float d = floor(color.r * levels + threshold) / levels;
    vec3 dithered = vec3(d);

    gl_FragColor = vec4(dithered, color.a);
}
