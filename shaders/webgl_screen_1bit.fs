#version 100
precision highp float;

// 1-bit screen pass: every pixel is either "ink" (palette0) or "paper"
// (palette3), with an 8x8 ordered (Bayer) dither standing in for the tones in
// between. Takes the same palette uniforms as webgl_screen_dithering.fs, so
// Palettes::apply works on it too; only the darkest and lightest colours are
// used.
//
// The same program draws two layers, picked by the `layer` uniform:
//   0 — the scene (default)
//   1 — the layer the search spotlight reveals: the sobel edge pass plus
//       citizens. Already two-tone, so no outlines (they'd ink over the lines).

uniform sampler2D screenTexture;
uniform float width;  // viewport size, set by render_framebuffer_as_quad
uniform float height;
uniform vec3 palette0; // ink (darkest)
uniform vec3 palette3; // paper (lightest)
uniform float layer;

varying vec2 TexCoords;

// Size of one 1-bit pixel in framebuffer pixels. 1.0 dithers at full
// resolution; 2.0 or 3.0 gives chunkier, more visible pixels.
const float PIXEL_SIZE = 2.0;
// Tone curve applied before dithering. 1-bit has no midtones to spare, so a
// bit of extra contrast keeps shapes readable instead of a uniform 50% grey.
const float CONTRAST = 1.35;
const float BRIGHTNESS = 0.05;
// Ink outlines where the (undithered) luminance changes sharply between
// neighbouring pixels, so building edges and silhouettes survive the
// dither. Set OUTLINE to false to turn them off.
const bool OUTLINE = true;
const float OUTLINE_THRESHOLD = 0.18;
// Citizen pixels carry a marker alpha (see markerAlpha in
// webgl_model_cooktor_flat.fs). Their colour is the one the player picked for
// them, so CITIZEN_MODE picks how much of it survives:
//   0 — their own colour, undithered (stands out from the 1-bit scene)
//   1 — dithered between ink and their own colour: same pixel texture as the
//       scene, but still colour-coded
//   2 — dithered like everything else (pure 1-bit, colours lost)
const int CITIZEN_MODE = 1;
// Mode 1: how far citizens' tone is lifted toward their colour before
// dithering, so dark-ish picks still show enough coloured pixels.
const float CITIZEN_LIFT = 0.35;

// 2x2 Bayer threshold, nested to get 4x4 and 8x8 without a lookup table
// (GLSL ES 1.00 has no const array initialisers).
float bayer2(vec2 a) {
    a = mod(floor(a), 2.0);
    return fract(a.x * 0.5 + a.y * a.y * 0.75);
}

float bayer4(vec2 a) {
    return bayer2(0.5 * a) * 0.25 + bayer2(a);
}

float bayer8(vec2 a) {
    return bayer4(0.5 * a) * 0.25 + bayer2(a);
}

float luma(vec3 c) {
    float l = dot(c, vec3(0.299, 0.587, 0.114));
    return clamp((l - 0.5) * CONTRAST + 0.5 + BRIGHTNESS, 0.0, 1.0);
}

vec4 sample_cell(vec2 cell, vec2 res) {
    return texture2D(screenTexture, (cell + 0.5) * PIXEL_SIZE / res);
}

void main() {
    vec2 res = vec2(width, height);
    // Snap to the 1-bit pixel grid: every fragment in a cell samples the
    // cell's centre and gets the same dither threshold. Works from TexCoords
    // rather than gl_FragCoord so the grid stays in viewport pixels when this
    // pass is blitted straight to a differently-sized window.
    vec2 cell = floor(TexCoords * res / PIXEL_SIZE);
    vec4 color = sample_cell(cell, res);
    // +0.5/64 centres the thresholds so pure black and pure white stay solid.
    float threshold = bayer8(cell) + 0.5 / 64.0;

    float lum = luma(color.rgb);

    if (CITIZEN_MODE != 2 && abs(color.a - 0.5) < 0.1) {
        if (CITIZEN_MODE == 0) {
            gl_FragColor = vec4(color.rgb, 1.0);
        } else {
            float tone = mix(lum, 1.0, CITIZEN_LIFT);
            gl_FragColor = vec4(tone < threshold ? palette0 : color.rgb, 1.0);
        }
        return;
    }

    bool ink = lum < threshold;

    if (OUTLINE && layer < 0.5) {
        float l = luma(sample_cell(cell + vec2(1.0, 0.0), res).rgb);
        float u = luma(sample_cell(cell + vec2(0.0, 1.0), res).rgb);
        if (max(abs(lum - l), abs(lum - u)) > OUTLINE_THRESHOLD)
            ink = true;
    }

    gl_FragColor = vec4(ink ? palette0 : palette3, color.a);
}
