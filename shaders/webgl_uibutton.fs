#version 100
precision mediump float;

// Clickable UI button panel. Drawn on a unit quad (webgl_decal.vs passes vUV in
// 0..1). uColor is the per-state base colour (idle/hover/press/flash/disabled,
// set per-frame by GameScene::set_button_color); uHalfSize is the button's
// half-extents in UI units (set once in make_button) so the corner rounding is
// circular regardless of the button's aspect. The panel gets rounded corners, a
// subtle top-to-bottom gradient, and a soft lighter rim for a little depth.
uniform vec4 uColor;
uniform vec2 uHalfSize;

varying vec2 vUV;

void main()
{
    // Local position from the centre, in UI units.
    vec2 p = (vUV - 0.5) * 2.0 * uHalfSize;
    float radius = min(min(uHalfSize.x, uHalfSize.y) * 0.6, 14.0);

    // Signed distance to a rounded rectangle (negative inside, 0 on the edge).
    vec2 d = abs(p) - (uHalfSize - vec2(radius));
    float dist = min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;

    // Antialiased coverage (~1.2px soft edge); drop fully-outside fragments.
    float aa = 1.2;
    float mask = 1.0 - smoothstep(-aa, aa, dist);
    if (mask <= 0.002)
        discard;

    // Gentle vertical gradient: a touch brighter at the top, darker at the bottom.
    float g = mix(1.14, 0.86, vUV.y);
    vec3 col = uColor.rgb * g;

    // Soft lighter rim just inside the edge for a beveled, raised look.
    float rim = smoothstep(-radius, -1.0, dist); // 0 in the middle -> 1 at the rim
    col += rim * 0.10;

    gl_FragColor = vec4(col, uColor.a * mask);
}
