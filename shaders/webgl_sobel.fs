#version 100
precision highp float;

// The search spotlight's outlines: white lines on the buildings' and floor's
// edges, black elsewhere. Drawn from the main pass's geometry attachments
// (written by webgl_model_sun.fs, see DitherOut / GeomOut there) rather than
// from a lit image, so the lines show the shapes alone: they don't change
// with the time of day or the lamps, and two faces lit the same still get
// their edge. Edges are found the way webgl_screen_1bit.fs finds its
// geometry outlines, but marked on both sides, so lines are two pixels wide.
//
// Bound by render_framebuffer_as_quad from the main pass's framebuffer:
//   screenTexture1: g = 1 on building/floor pixels, b = depth high byte
//   screenTexture2: rg = octahedral normal, b = depth low byte
uniform sampler2D screenTexture1;
uniform sampler2D screenTexture2;
uniform float width;
uniform float height;

varying vec2 TexCoords;

// A line where the depth jumps by more than DEPTH_EDGE world units (beyond
// what the surface's own slope explains), or where neighbouring normals are
// more than about acos(NORMAL_EDGE) apart. DEPTH_RANGE is the camera's depth
// range; keep it the same as in webgl_screen_1bit.fs.
const float DEPTH_EDGE = 6.0;
const float NORMAL_EDGE = 0.85;
const float DEPTH_RANGE = 20000.0;

float depth_of(vec4 g1, vec4 g2) {
    float hi = floor(g1.b * 255.0 + 0.5);
    float lo = floor(g2.b * 255.0 + 0.5);
    return (hi * 256.0 + lo) / 65535.0 * DEPTH_RANGE;
}

vec3 normal_of(vec4 g2) {
    vec2 p = g2.rg * 2.0 - 1.0;
    vec3 n = vec3(p, 1.0 - abs(p.x) - abs(p.y));
    if (n.z < 0.0) {
        vec2 s = vec2(n.x >= 0.0 ? 1.0 : -1.0, n.y >= 0.0 ? 1.0 : -1.0);
        n.xy = (1.0 - abs(n.yx)) * s;
    }
    return normalize(n);
}

bool edge(vec2 uv, vec2 px) {
    vec4 p1 = texture2D(screenTexture1, uv);
    // Nothing (the background) here: no line.
    if (p1.g <= 0.5)
        return false;

    vec2 l = uv - vec2(px.x, 0.0), r = uv + vec2(px.x, 0.0);
    vec2 d = uv - vec2(0.0, px.y), u = uv + vec2(0.0, px.y);
    vec4 l1 = texture2D(screenTexture1, l), r1 = texture2D(screenTexture1, r);
    vec4 d1 = texture2D(screenTexture1, d), u1 = texture2D(screenTexture1, u);
    // Next to the background.
    if (l1.g <= 0.5 || r1.g <= 0.5 || d1.g <= 0.5 || u1.g <= 0.5)
        return true;

    vec4 p2 = texture2D(screenTexture2, uv);
    vec4 l2 = texture2D(screenTexture2, l), r2 = texture2D(screenTexture2, r);
    vec4 d2 = texture2D(screenTexture2, d), u2 = texture2D(screenTexture2, u);

    // On a plane dl + dr - 2*dp is ~0; on either side of a silhouette it's
    // about the size of the jump.
    float dp = depth_of(p1, p2);
    float dl = depth_of(l1, l2), dr = depth_of(r1, r2);
    float dd = depth_of(d1, d2), du = depth_of(u1, u2);
    if (abs(dl + dr - 2.0 * dp) > DEPTH_EDGE || abs(dd + du - 2.0 * dp) > DEPTH_EDGE)
        return true;

    vec3 np = normal_of(p2);
    return dot(np, normal_of(l2)) < NORMAL_EDGE || dot(np, normal_of(r2)) < NORMAL_EDGE ||
           dot(np, normal_of(d2)) < NORMAL_EDGE || dot(np, normal_of(u2)) < NORMAL_EDGE;
}

void main(void)
{
    vec2 px = vec2(1.0 / width, 1.0 / height);
    gl_FragColor = vec4(edge(TexCoords, px) ? vec3(1.0) : vec3(0.0), 1.0);
}
