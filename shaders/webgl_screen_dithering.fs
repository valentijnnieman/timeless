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

vec3 pick(float idx) {
    if (idx < 0.5)       return palette0;
    else if (idx < 1.5)  return palette1;
    else if (idx < 2.5)  return palette2;
    return palette3;
}

void main() {
    vec2 pixelPos = gl_FragCoord.xy;
    vec4 color = texture2D(screenTexture, TexCoords);

    float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    float threshold = bayer4(pixelPos);
    float idx = clamp(floor(lum * 4.0 + threshold), 0.0, 3.0);

    vec3 result = pick(idx);

    // Citizen pixels carry a marker alpha (see markerAlpha in
    // webgl_model_cooktor_flat.fs), so they can be kept readable against
    // geometry that lands in the same luminance bucket (e.g. a white citizen on
    // a light bridge deck). CITIZEN_TRUE_COLORS picks how:
    //   true  — skip the dither entirely and show the citizen's own colour
    //   false — keep the palette, pulled halfway toward the neighbouring tone
    const bool CITIZEN_TRUE_COLORS = true;
    if (abs(color.a - 0.5) < 0.1) {
        if (CITIZEN_TRUE_COLORS) {
            gl_FragColor = vec4(color.rgb, 1.0);
        } else {
            vec3 neighbour = idx < 0.5 ? pick(1.0) : pick(idx - 1.0);
            gl_FragColor = vec4(mix(result, neighbour, 0.5), 1.0);
        }
        return;
    }

    gl_FragColor = vec4(result, color.a);
}
