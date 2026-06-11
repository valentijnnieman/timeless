#version 300 es
precision highp float;
// Water surface fragment shader (beach) — simple, calm, and slightly see-through.
// Flat-lit water with a slow large-scale brightness shimmer, a soft sun
// highlight, and a gently animated transparency. Relies on the global alpha
// blend (window_manager) plus the water entity's set_entity_alpha translucency
// (no depth write) so it blends over the sand without z-fighting.

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3  lightPos;        // directional light: direction pointing toward the sun
uniform vec3  lightColor;
uniform vec3  cameraPos;
uniform float ambientStrength;
uniform float time;
uniform sampler2D normalMap;   // unit 2 — tangent-space ripple normal map

out vec4 fragColor;

const vec3  WATER     = vec3(0.10, 0.42, 0.55); // base water color (tweak to taste)
const float ALPHA_MIN = 0.85;                   // most transparent
const float ALPHA_MAX = 0.95;                   // most opaque

// Animated wave normal-mapping. The surface is a flat plane facing world-up, so
// its tangent frame is just T=+X, B=+Y, N=+Z and a sampled tangent-space normal
// maps straight into world space. The map is sampled ONCE across the whole plane
// (no tiling) and scrolled over time so the waves roll along the surface. Unlike
// the sand, the water map is used in its native orientation (no 90 deg turn).
// STRENGTH scales the sideways tilt (0 = flat water, higher = choppier).
const vec2  SCROLL     = vec2(0.04, 0.025); // UV/sec drift of the wave texture
const float N_STRENGTH = 0.6;

void main()
{
    // One copy of the map across the object, drifting over time.
    vec2 uv = TexCoord + SCROLL * time;
    vec3 nTS = texture(normalMap, uv).rgb * 2.0 - 1.0;
    vec3 N = normalize(vec3(nTS.xy * N_STRENGTH, 1.0)); // T=+X, B=+Y, up=+Z
    vec3 V = normalize(cameraPos - FragPos);
    vec3 L = normalize(-lightPos);
    float ndl = max(dot(N, L), 0.0);

    // Slow, large-scale shimmer so the surface looks alive without sharp banding.
    float shimmer = 0.5 + 0.5 * sin(FragPos.x * 0.015 + FragPos.y * 0.01 + time * 0.8);
    vec3 base = WATER * (0.85 + 0.15 * shimmer);

    // Sharper, ripple-driven sun highlight (higher exponent so the normal map's
    // tilt breaks the broad glare into moving glints).
    vec3 H = normalize(V + L);
    float spec = pow(max(dot(N, H), 0.0), 64.0) * 0.5;

    vec3 color = base * (ambientStrength + ndl) + lightColor * spec;

    // Match the scene's Reinhard tonemap + gamma.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    // Gently breathing transparency (mixes with the shimmer for subtle variation).
    float a = mix(ALPHA_MIN, ALPHA_MAX, 0.5 + 0.5 * sin(time * 0.5));
    a = clamp(a + (shimmer - 0.5) * 0.08, 0.0, 1.0);
    fragColor = vec4(color, a);
}
