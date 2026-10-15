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
//
// Where the scene has a world-space dither threshold (screenTexture1, written
// by the building/floor shader webgl_model_sun.fs) the pixel is dithered
// against that instead, Obra Dinn style: the pattern is fixed to the world,
// so it stays on the buildings when the camera pans and scales with zoom
// (SCREEN_ALIGNED there picks how it's laid out). Those pixels aren't snapped
// to the PIXEL_SIZE grid; the world cell size (DITHER_CELL there) sets how
// chunky they look. Everything else
// (citizens, sprites, background) uses the screen pattern below.

uniform sampler2D screenTexture;
// Written by the building/floor shader (see DitherOut / GeomOut in
// webgl_model_sun.fs), bound by render_framebuffer_as_quad from the
// framebuffer's extra colour attachments. Both read 0 where nothing wrote
// them, and when there are none (layer 1).
//   1: r = world-space threshold, g = 1 on building/floor pixels,
//      b = depth high byte
//   2: rg = octahedral normal, b = depth low byte
uniform sampler2D screenTexture1;
uniform sampler2D screenTexture2;
uniform float width;  // viewport size, set by render_framebuffer_as_quad
uniform float height;
uniform vec3 palette0; // ink (darkest)
uniform vec3 palette3; // paper (lightest)
uniform float layer;
// The world origin's position in the framebuffer, in pixels (set each frame by
// the game). The screen pattern's grid is laid out from here rather than from
// the screen corner, so it pans along with the map too.
uniform vec2 ditherOrigin;

varying vec2 TexCoords;

// Size of one 1-bit pixel in framebuffer pixels. 1.0 dithers at full
// resolution; 2.0 or 3.0 gives chunkier, more visible pixels.
const float PIXEL_SIZE = 1.0;
// Tone curve for outlines and citizens. 1-bit has no midtones to spare, so a
// bit of extra contrast keeps shapes readable instead of a uniform 50% grey.
const float CONTRAST = 1.35;
const float BRIGHTNESS = 0.05;
// Levels for the scene's dither: luminance at or below INK_LEVEL is solid ink,
// at or above PAPER_LEVEL solid paper, and only the tones in between dither.
// Flat-lit faces then read as solid black or white, and the dots are left for
// the transitions (light falloff, shading), Obra Dinn style. Pull the two
// together for harder black and white, apart for more dither.
const float INK_LEVEL = 0.35;
const float PAPER_LEVEL = 0.75;
// Ink outlines where the (undithered) luminance changes sharply between
// neighbouring pixels, so building edges and silhouettes survive the
// dither. Set OUTLINE to false to turn them off. Buildings and floor use the
// geometry outlines below instead (when GEOMETRY_OUTLINE is on).
const bool OUTLINE = true;
const float OUTLINE_THRESHOLD = 0.18;
// One-pixel lines on the buildings' and floor's actual edges, from their depth
// and normals rather than their brightness, so every silhouette and crease is
// drawn even where both sides are lit the same:
//   - silhouettes: the nearer side of a depth jump of more than DEPTH_EDGE
//     world units (beyond what the surface's own slope explains)
//   - creases: neighbouring normals more than about acos(NORMAL_EDGE) apart
//   - against the background (anything that isn't building or floor)
const bool GEOMETRY_OUTLINE = true;
const float DEPTH_EDGE = 6.0;
const float NORMAL_EDGE = 0.85;
// The camera's depth range (its ortho near/far planes, -10000..10000), to turn
// the stored 0..1 depth back into world units.
const float DEPTH_RANGE = 20000.0;
// Line colour:
//   0 — always ink
//   1 — the opposite of the surface's tone: paper lines on dark surfaces,
//       ink on lit ones, so a solid-black area still shows its edges as
//       single white lines (Obra Dinn style)
const int LINE_MODE = 1;
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

// The tone the scene is dithered with (see INK_LEVEL / PAPER_LEVEL).
float tone(vec3 c) {
    float l = dot(c, vec3(0.299, 0.587, 0.114));
    return clamp((l - INK_LEVEL) / (PAPER_LEVEL - INK_LEVEL), 0.0, 1.0);
}

bool is_world(vec2 uv) {
    return texture2D(screenTexture1, uv).g > 0.5;
}

float depth_at(vec2 uv) {
    float hi = floor(texture2D(screenTexture1, uv).b * 255.0 + 0.5);
    float lo = floor(texture2D(screenTexture2, uv).b * 255.0 + 0.5);
    return (hi * 256.0 + lo) / 65535.0 * DEPTH_RANGE;
}

vec3 normal_at(vec2 uv) {
    vec2 p = texture2D(screenTexture2, uv).rg * 2.0 - 1.0;
    vec3 n = vec3(p, 1.0 - abs(p.x) - abs(p.y));
    if (n.z < 0.0) {
        vec2 s = vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
        n.xy = (1.0 - abs(n.yx)) * s;
    }
    return normalize(n);
}

// Whether this building/floor pixel sits on a geometry edge (see
// GEOMETRY_OUTLINE). Lines stay one pixel wide: silhouettes only mark the
// nearer side, creases only compare with the right and upper neighbour.
bool geometry_edge(vec2 uv, vec2 px) {
    vec2 l = uv - vec2(px.x, 0.0), r = uv + vec2(px.x, 0.0);
    vec2 d = uv - vec2(0.0, px.y), u = uv + vec2(0.0, px.y);
    if (!is_world(l) || !is_world(r) || !is_world(d) || !is_world(u))
        return true;

    // A depth jump only counts if the surface's own slope doesn't explain
    // it: on a plane dl + dr - 2*dp is ~0, at the near side of a silhouette
    // it's about the size of the jump.
    float dp = depth_at(uv);
    float dl = depth_at(l), dr = depth_at(r);
    float dd = depth_at(d), du = depth_at(u);
    if (dl + dr - 2.0 * dp > DEPTH_EDGE && max(dl, dr) - dp > DEPTH_EDGE)
        return true;
    if (dd + du - 2.0 * dp > DEPTH_EDGE && max(dd, du) - dp > DEPTH_EDGE)
        return true;

    vec3 np = normal_at(uv);
    return dot(np, normal_at(r)) < NORMAL_EDGE || dot(np, normal_at(u)) < NORMAL_EDGE;
}

vec4 sample_cell(vec2 cell, vec2 res, vec2 origin) {
    return texture2D(screenTexture, ((cell + 0.5) * PIXEL_SIZE + origin) / res);
}

void main() {
    vec2 res = vec2(width, height);
    // Snap to the 1-bit pixel grid: every fragment in a cell samples the
    // cell's centre and gets the same dither threshold. Works from TexCoords
    // rather than gl_FragCoord so the grid stays in viewport pixels when this
    // pass is blitted straight to a differently-sized window. The grid is
    // anchored at ditherOrigin, wrapped to one 8x8 pattern repeat so the cell
    // numbers stay small (the pattern looks the same either way).
    vec2 origin = mod(ditherOrigin, 8.0 * PIXEL_SIZE);
    vec2 cell = floor((TexCoords * res - origin) / PIXEL_SIZE);
    vec4 color = sample_cell(cell, res, origin);
    // +0.5/64 centres the thresholds so pure black and pure white stay solid.
    float threshold = bayer8(cell) + 0.5 / 64.0;

    // World-space threshold: per framebuffer pixel, no grid snapping.
    vec4 world = texture2D(screenTexture1, TexCoords);
    bool world_space = layer < 0.5 && world.g > 0.5;
    if (world_space) {
        color = texture2D(screenTexture, TexCoords);
        threshold = world.r;
    }

    float lum = luma(color.rgb);

    if (CITIZEN_MODE != 2 && abs(color.a - 0.5) < 0.1) {
        if (CITIZEN_MODE == 0) {
            gl_FragColor = vec4(color.rgb, 1.0);
        } else {
            float citizen_tone = mix(lum, 1.0, CITIZEN_LIFT);
            gl_FragColor = vec4(citizen_tone < threshold ? palette0 : color.rgb, 1.0);
        }
        return;
    }

    float t = tone(color.rgb);
    bool ink = t < threshold;

    if (world_space && GEOMETRY_OUTLINE) {
        if (geometry_edge(TexCoords, 1.0 / res))
            ink = LINE_MODE == 0 || t >= 0.5;
    } else if (OUTLINE && layer < 0.5) {
        if (world_space) {
            // Neighbours on all four sides, so the line is two pixels wide
            // (one each side of the edge) like the PIXEL_SIZE outlines.
            vec2 px = 1.0 / res;
            float d = max(
                max(abs(lum - luma(texture2D(screenTexture, TexCoords + vec2(px.x, 0.0)).rgb)),
                    abs(lum - luma(texture2D(screenTexture, TexCoords - vec2(px.x, 0.0)).rgb))),
                max(abs(lum - luma(texture2D(screenTexture, TexCoords + vec2(0.0, px.y)).rgb)),
                    abs(lum - luma(texture2D(screenTexture, TexCoords - vec2(0.0, px.y)).rgb))));
            if (d > OUTLINE_THRESHOLD)
                ink = true;
        } else {
            float l = luma(sample_cell(cell + vec2(1.0, 0.0), res, origin).rgb);
            float u = luma(sample_cell(cell + vec2(0.0, 1.0), res, origin).rgb);
            if (max(abs(lum - l), abs(lum - u)) > OUTLINE_THRESHOLD)
                ink = true;
        }
    }

    gl_FragColor = vec4(ink ? palette0 : palette3, color.a);
}
