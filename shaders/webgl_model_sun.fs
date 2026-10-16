#version 300 es
precision highp float;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
// World position in camera axes, without the pan (set by the vertex shader).
in vec2 DitherPos;

uniform vec3 materialDiffuse;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;

#define MAX_POINT_LIGHTS 32
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

// Point lights (street lamps) fade out smoothly over this distance instead of
// stopping dead, so they leave pools of light that the 1-bit pass dithers
// into the dark around them. POINT_LIGHT_STRENGTH scales their brightness.
const float POINT_LIGHT_RADIUS = 256.0;
const float POINT_LIGHT_STRENGTH = 1.0;
// How much the models' material colours count. In 1-bit only their
// brightness survives, and a dark material (amshouse4's maroon walls are 0.04)
// can never be lit above INK_LEVEL, so those buildings stay black whatever
// light hits them. This mixes every material toward white:
//   0 — ignore the material colours: every surface white, shaded only by the
//       lighting (details like window frames then only show through the
//       geometry outlines)
//   1 — the colours as modelled
// In between, dark materials are lifted while light and dark keep their order,
// so details keep some contrast and every building responds to light.
const float ALBEDO_INFLUENCE = 1.0;
// Water: meshes the game tags with materialKind 1 (Mesh::kind; the canals'
// "water" material, and the canal bed seen under bridges). Instead of its
// material colour it gets WATER_TONE: light enough to read near-white by day,
// black at night. Lamps hardly light it (WATER_LAMP_DIFFUSE: water reflects
// rather than scatters), so at night their light shows as glints on dark
// water. Two animated effects, sized in screen pixels and laid out along the
// screen (horizontal whatever way the camera faces):
//   - ripples: short wavy lines, RIPPLE_SPACING px apart, drifting: ink on the
//     light daytime water; on dark water they catch the lamps instead, as
//     white streaks fading out with each lamp's light (RIPPLE_LAMP_REACH)
//   - glints: twinkling white sparkles where a lamp (or a low sun) reflects
//     off the rippled surface (they show on the dark night water)
uniform int materialKind;
uniform float time; // seconds, set by the render
const float WATER_TONE = 0.45;
const float WATER_LAMP_DIFFUSE = 0.15;
const float RIPPLE_SPACING = 7.0;  // px between ripple lines
const float RIPPLE_WIDTH = 1.0;    // px
const float RIPPLE_WAVE = 2.0;     // px the lines wave up and down
const float RIPPLE_SPEED = 0.6;    // how fast they move
const float RIPPLE_GAPS = 0.35;    // share of each line broken into gaps
const float RIPPLE_LAMP_REACH = 1.0; // how far lit ripples reach (x lamp radius)
const float GLINT_SHARPNESS = 60.0; // higher = smaller, rarer reflections
const float GLINT_TILT = 0.35;     // how choppy the surface is for glints
const float GLINT_TWINKLE = 0.25;  // share of 2px cells that may sparkle
const float GLINT_RATE = 5.0;      // sparkle changes per second
// Street: meshes tagged materialKind 2 (the ground tiles). Neither sun nor
// ambient light reaches it, so it stays black all day, keeping whatever's
// drawn on it (the follow arrow, footprints) readable; but the lamps light it
// as if it were STREET_LAMP_TONE, leaving pools of light around them.
const float STREET_LAMP_TONE = 0.8;
// Fake ambient occlusion on walls: they darken toward the ground, so a facade
// shades gradually instead of being one flat tone (which dithers into an even
// field of dots). WALL_GROUND_Z is where the map's tiles stand; walls are at
// WALL_AO_DARK of their light there, back to full by WALL_AO_HEIGHT above it.
const float WALL_GROUND_Z = 10.0;
const float WALL_AO_HEIGHT = 96.0;
const float WALL_AO_DARK = 0.45;

layout(location = 0) out vec4 FragColorOut;
// Data for the 1-bit screen pass (webgl_screen_1bit.fs). Only lands when the
// framebuffer has these attachments enabled; ignored otherwise. Alpha stays 1
// in both, since the scene pass blends and would mix any other value in.
//   DitherOut: r = dither threshold, g = 1 (marks a building/floor pixel),
//              b = depth, high byte
//   GeomOut:   rg = surface normal (octahedral), b = depth, low byte
// Normal and depth are what the screen pass draws its outlines from.
layout(location = 1) out vec4 DitherOut;
layout(location = 2) out vec4 GeomOut;

// Which threshold pattern the cells use:
//   0 — 8x8 Bayer: regular, reads as a grid of + and x shapes
//   1 — blue noise (ditherNoise, a 64x64 void-and-cluster map from
//       tools/generate_blue_noise.py): evenly spread but irregular dots, more
//       like grain or stippling
const int PATTERN = 1;
// The blue-noise map. Bound to texture unit 7 by the game every frame.
uniform highp sampler2D ditherNoise;

// Side of one dither cell, in framebuffer pixels.
// Whole numbers keep the cells on whole pixels at zoom 1.
const float DITHER_CELL = 2.0;
// The camera zoom (set by the game each frame). One framebuffer pixel is
// `zoom` world units, so scaling the cells by this keeps the dots DITHER_CELL
// pixels on screen however far you zoom, instead of growing like a texture.
// 0 (never set) counts as 1.
uniform float ditherScale;
// World offset of the pattern (set by the game). It only changes while
// zooming, so that the pattern scales about the middle of the view rather than
// the world origin: zooming makes it breathe in place instead of sweeping
// across the map. Panning and turning leave it alone, so the pattern stays on
// the surfaces then. Also used by the vertex shaders for DitherPos.
uniform vec3 ditherOffset;

// Same nested 2x2 Bayer matrix as the screen pass.
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

// Where the pattern is laid out. Either way it stays put on the buildings and
// floor when the camera pans, and scales with zoom.
//   true  — on the world position seen along the camera (Obra Dinn style):
//           square cells lined up with the screen pixels, one continuous
//           pattern across walls and floor. No moire from the iso skew, but
//           the pattern slides over the buildings whenever the camera turns
//           or orbits (Q/E, main menu).
//   false — on each surface, from the world axes it faces (triplanar): the
//           pattern is printed on every face and stays put through turns and
//           orbits too, but the iso camera skews its cells, which can alias
//           into diagonal stripes (bigger DITHER_CELL helps).
const bool SCREEN_ALIGNED = false;

float world_dither(vec3 pos, vec3 n) {
    pos += ditherOffset;
    vec2 uv;
    if (SCREEN_ALIGNED) {
        uv = DitherPos;
    } else {
        vec3 an = abs(n);
        if (an.z >= an.x && an.z >= an.y)
            uv = pos.xy;
        else if (an.x >= an.y)
            uv = pos.yz;
        else
            uv = pos.xz;
    }
    float scale = ditherScale > 0.0 ? ditherScale : 1.0;
    vec2 cell = floor(uv / (DITHER_CELL * scale));
    if (PATTERN == 1) {
        // The map tiles, so wrap the cell into it (mod is floor-based, so
        // negative cells wrap correctly too). +0.5/256 centres the 256 levels
        // like the Bayer offset below.
        ivec2 texel = ivec2(mod(cell, 64.0));
        return texelFetch(ditherNoise, texel, 0).r + 0.5 / 256.0;
    }
    // +0.5/64 centres the thresholds so pure black and white stay solid.
    return bayer8(cell) + 0.5 / 64.0;
}

// Octahedral normal encoding: a unit vector in two 0..1 values, so it fits
// two 8-bit channels with even precision in every direction.
vec2 sign_not_zero(vec2 v) {
    return vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

vec2 encode_normal(vec3 n) {
    n /= abs(n.x) + abs(n.y) + abs(n.z);
    vec2 p = n.z >= 0.0 ? n.xy : (1.0 - abs(n.yx)) * sign_not_zero(n.xy);
    return p * 0.5 + 0.5;
}

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Ripple lines and glints on top of the water's lit colour (see WATER_TONE).
vec3 water_surface(vec3 color, vec3 sunDir) {
    // Screen-aligned position in pixels: DitherPos is the world position in
    // camera axes (fixed to the map when panning), / zoom = pixels.
    float scale = ditherScale > 0.0 ? ditherScale : 1.0;
    vec2 p = DitherPos / scale;

    // Ripples: rows of wavy lines, each row with its own phase, broken
    // into dashes that drift sideways.
    float row = floor(p.y / RIPPLE_SPACING);
    float wave = sin(p.x * 0.05 + row * 1.7 + time * RIPPLE_SPEED) * RIPPLE_WAVE;
    bool on_line = mod(p.y + wave, RIPPLE_SPACING) < RIPPLE_WIDTH;
    bool in_dash = sin(p.x * 0.11 + row * 3.1 - time * RIPPLE_SPEED * 0.7)
                   > RIPPLE_GAPS * 2.0 - 1.0;
    // How much lamp light reaches this bit of water (0..1), for the night
    // ripples below.
    float lamp = 0.0;
    for (int i = 0; i < numPointLights; ++i) {
        float d = clamp(length(pointLightPositions[i] - FragPos)
                        / (POINT_LIGHT_RADIUS * RIPPLE_LAMP_REACH), 0.0, 1.0);
        lamp += (1.0 - d * d) * (1.0 - d * d);
    }
    lamp = clamp(lamp * POINT_LIGHT_STRENGTH, 0.0, 1.0);
    if (on_line && in_dash) {
        // Light water: an ink line. Dark water: the line catches the lamps.
        bool dark = dot(color, vec3(0.299, 0.587, 0.114)) < 0.5;
        color = dark ? mix(color, vec3(1.0), lamp) : color * 0.15;
    }

    // Glints: a choppy normal from a few moving sine waves, lit with a sharp
    // specular from the sun and the lamps, shown only on twinkling cells.
    vec2 q = p * 0.2;
    vec2 tilt = vec2(sin(q.x * 1.1 + time * 1.3) * cos(q.y * 0.9 - time * 0.9),
                     cos(q.x * 0.7 - time * 1.1) * sin(q.y * 1.2 + time * 0.7));
    vec3 n = normalize(vec3(tilt * GLINT_TILT, 1.0));
    vec3 v = normalize(cameraPos - FragPos);
    float spec = pow(max(dot(n, normalize(sunDir + v)), 0.0), GLINT_SHARPNESS)
                 * dot(lightColor, vec3(0.333));
    for (int i = 0; i < numPointLights; ++i) {
        vec3 toLight = pointLightPositions[i] - FragPos;
        float d = clamp(length(toLight) / POINT_LIGHT_RADIUS, 0.0, 1.0);
        float falloff = (1.0 - d * d) * (1.0 - d * d);
        vec3 l = normalize(toLight);
        spec += pow(max(dot(n, normalize(l + v)), 0.0), GLINT_SHARPNESS * 0.5)
                * falloff * POINT_LIGHT_STRENGTH;
    }
    float twinkle = hash(floor(p / 2.0) + floor(time * GLINT_RATE) * 17.0);
    if (twinkle < GLINT_TWINKLE && spec > 0.3)
        color = vec3(1.0);
    return color;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos);
    bool water = materialKind == 1;
    bool street = materialKind == 2;
    vec3 albedo = water ? vec3(WATER_TONE)
                        : mix(vec3(1.0), materialDiffuse, ALBEDO_INFLUENCE);
    vec3 lampAlbedo = street ? vec3(STREET_LAMP_TONE) : albedo;

    vec3 ambient = street ? vec3(0.0) : ambientStrength * albedo;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = street ? vec3(0.0) : diff * albedo * lightColor;

    vec3 pointDiffuse = vec3(0.0);
    for (int i = 0; i < numPointLights; ++i) {
        vec3 toLight = pointLightPositions[i] - FragPos;
        float dist = length(toLight);
        // Smooth falloff to zero at the radius, staying bright for longer
        // than a plain quadratic: about half strength halfway out, then a
        // soft edge rather than a hard disc.
        float d = clamp(dist / POINT_LIGHT_RADIUS, 0.0, 1.0);
        float falloff = 1.0 - d * d;
        float attenuation = falloff * falloff * POINT_LIGHT_STRENGTH;
        vec3 plDir = normalize(toLight);
        float plDiff = max(dot(norm, plDir), 0.0);
        pointDiffuse += plDiff * attenuation * lampAlbedo * pointLightColors[i];
    }

    if (water)
        pointDiffuse *= WATER_LAMP_DIFFUSE;
    vec3 result = ambient + diffuse + pointDiffuse;

    // Walls only (weighted by how vertical the face is), so roofs and the
    // floor keep their tone.
    float height = clamp((FragPos.z - WALL_GROUND_Z) / WALL_AO_HEIGHT, 0.0, 1.0);
    float ao = mix(WALL_AO_DARK, 1.0, smoothstep(0.0, 1.0, height));
    result *= mix(1.0, ao, 1.0 - abs(norm.z));
    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    if (water)
        gammaCorrected = water_surface(gammaCorrected, lightDir);
    FragColorOut = vec4(gammaCorrected, 1.0);
    // Window depth (linear for the ortho camera) as 16 bits over two bytes.
    float depth = floor(gl_FragCoord.z * 65535.0 + 0.5);
    float depth_hi = floor(depth / 256.0);
    float depth_lo = depth - depth_hi * 256.0;
    DitherOut = vec4(world_dither(FragPos, norm), 1.0, depth_hi / 255.0, 1.0);
    GeomOut = vec4(encode_normal(norm), depth_lo / 255.0, 1.0);
}
