#version 100
precision highp float;

uniform sampler2D screenTexture;
uniform vec3 palette0; // darkest
uniform vec3 palette1;
uniform vec3 palette2;
uniform vec3 palette3; // lightest

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

    float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    float threshold = bayer4(pixelPos);
    float idx = clamp(floor(lum * 4.0 + threshold), 0.0, 3.0);

    vec3 result;
    if (idx < 0.5)       result = palette0;
    else if (idx < 1.5)  result = palette1;
    else if (idx < 2.5)  result = palette2;
    else                 result = palette3;

    gl_FragColor = vec4(result, color.a);
}
