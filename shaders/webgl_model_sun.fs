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
const float DITHER_CELL = 1.0;
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

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos);
    vec3 albedo = mix(vec3(1.0), materialDiffuse, ALBEDO_INFLUENCE);

    vec3 ambient = ambientStrength * albedo;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

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
        pointDiffuse += plDiff * attenuation * albedo * pointLightColors[i];
    }

    vec3 result = ambient + diffuse + pointDiffuse;

    // Walls only (weighted by how vertical the face is), so roofs and the
    // floor keep their tone.
    float height = clamp((FragPos.z - WALL_GROUND_Z) / WALL_AO_HEIGHT, 0.0, 1.0);
    float ao = mix(WALL_AO_DARK, 1.0, smoothstep(0.0, 1.0, height));
    result *= mix(1.0, ao, 1.0 - abs(norm.z));
    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    FragColorOut = vec4(gammaCorrected, 1.0);
    // Window depth (linear for the ortho camera) as 16 bits over two bytes.
    float depth = floor(gl_FragCoord.z * 65535.0 + 0.5);
    float depth_hi = floor(depth / 256.0);
    float depth_lo = depth - depth_hi * 256.0;
    DitherOut = vec4(world_dither(FragPos, norm), 1.0, depth_hi / 255.0, 1.0);
    GeomOut = vec4(encode_normal(norm), depth_lo / 255.0, 1.0);
}
