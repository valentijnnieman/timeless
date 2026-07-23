#version 100
precision mediump float;

// Animated backdrop: five colored control lines, each following its own sine
// curve and drifting left-to-right over time. Rather than drawing them as
// thin strokes, every pixel is colored by a gradient between whichever two
// lines bound it vertically — so the full canvas height is continuously
// covered by smoothly blending color bands instead of stripes of background
// showing between them. Drawn once per frame behind the 3D scene (see
// WindowManager::render_background_quad).
//
// Each line also gets a randomized "depth" in [0,1] (see hash11 below) that
// drives parallax speed and a slight fog fade — together these read as the
// bands sitting at different distances rather than a flat 2D overlay.

varying vec2 TexCoords;

uniform float time;

#define TAU 6.28318530718

// Uploaded once from BackgroundPalette::active (background_palette.hpp) via
// Resource::bg_waves_shader in resource.hpp — swap the palette there, not
// here, to keep this shader palette-agnostic. Ideally one color family (e.g.
// gold -> rust) rather than five distinct hues: the lines sort and blend
// into whichever neighbor is adjacent each frame, so staying within one
// family keeps every possible pairing calm and cohesive instead of reading
// as a rainbow.
uniform vec3 waveColorA;
uniform vec3 waveColorB;
uniform vec3 waveColorC;
uniform vec3 waveColorD;
uniform vec3 waveColorE;

// Cheap deterministic hash — turns a fixed per-line seed into a stable
// pseudo-random depth, so the parallax/fog spread below looks organic instead
// of hand-tuned, without any per-frame randomness (which would just flicker).
float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

// Returns this line's current y position (its color is computed by the
// caller). depth is randomized per-line: far lines (low depth) drift slower,
// near lines (high depth) drift faster — a simple parallax cue.
float waveY(float x, float freq, float speedBase, float phase, float centerY,
           float ampBase, float seed, out float depth) {
    depth = hash11(seed);
    float speed = speedBase * mix(0.8, 1.8, depth);
    float amp = ampBase * mix(0.6, 1.3, depth);
    return centerY + amp * sin(x * freq * TAU - time * speed + phase);
}

// Swaps (y, color) pairs so ya <= yb — used to sort the five lines by height
// each frame, since they drift independently and can cross one another.
void cmpswap(inout float ya, inout vec3 ca, inout float yb, inout vec3 cb) {
    if (ya > yb) {
        float ty = ya; ya = yb; yb = ty;
        vec3 tc = ca; ca = cb; cb = tc;
    }
}

void main() {
    vec2 uv = TexCoords;

    float d1, d2, d3, d4, d5;
    float y1 = waveY(uv.x, 0.8, 0.42, 0.0, 0.18, 0.08, 11.0, d1);
    float y2 = waveY(uv.x, 1.3, 0.68, 2.1, 0.36, 0.09, 27.0, d2);
    float y3 = waveY(uv.x, 0.5, 0.26, 4.4, 0.52, 0.07, 42.0, d3);
    float y4 = waveY(uv.x, 1.0, 0.52, 1.2, 0.68, 0.08, 58.0, d4);
    float y5 = waveY(uv.x, 0.65, 0.34, 3.0, 0.85, 0.06, 73.0, d5);

    // Fog: far lines fade slightly toward white, near lines stay fully saturated.
    vec3 c1 = mix(vec3(1.0), waveColorA, mix(0.75, 1.0, d1));
    vec3 c2 = mix(vec3(1.0), waveColorB, mix(0.75, 1.0, d2));
    vec3 c3 = mix(vec3(1.0), waveColorC, mix(0.75, 1.0, d3));
    vec3 c4 = mix(vec3(1.0), waveColorD, mix(0.75, 1.0, d4));
    vec3 c5 = mix(vec3(1.0), waveColorE, mix(0.75, 1.0, d5));

    // 9-comparator optimal sorting network for 5 elements, keeping each y
    // paired with its color as they get reordered.
    cmpswap(y1, c1, y2, c2);
    cmpswap(y4, c4, y5, c5);
    cmpswap(y3, c3, y5, c5);
    cmpswap(y3, c3, y4, c4);
    cmpswap(y1, c1, y4, c4);
    cmpswap(y1, c1, y3, c3);
    cmpswap(y2, c2, y5, c5);
    cmpswap(y2, c2, y4, c4);
    cmpswap(y2, c2, y3, c3);

    // Fill every pixel: above the topmost line and below the bottommost line
    // hold that line's color solid; everywhere in between blends smoothly
    // toward whichever neighbor line is next.
    vec3 color;
    if (uv.y < y1) {
        color = c1;
    } else if (uv.y < y2) {
        color = mix(c1, c2, (uv.y - y1) / max(y2 - y1, 0.0001));
    } else if (uv.y < y3) {
        color = mix(c2, c3, (uv.y - y2) / max(y3 - y2, 0.0001));
    } else if (uv.y < y4) {
        color = mix(c3, c4, (uv.y - y3) / max(y4 - y3, 0.0001));
    } else if (uv.y < y5) {
        color = mix(c4, c5, (uv.y - y4) / max(y5 - y4, 0.0001));
    } else {
        color = c5;
    }

    gl_FragColor = vec4(color, 0.45);
}
