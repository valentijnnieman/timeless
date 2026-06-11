#version 100
precision mediump float;

// Cursor reticle: a faint, hollow ring marking the cursor's true ground point
// under the floating bottle. Deliberately subtle (thin, soft, low alpha, neutral
// colour) so it reads as a quiet aim marker, not a bold decal like webgl_decal.
varying vec2 vUV;

void main()
{
    float d = length(vUV - vec2(0.5)) * 2.0; // 0 centre, 1 edge
    // A thin soft band near the rim: fade in, then back out, leaving the centre
    // empty (hollow) and the outer edge feathered.
    float ring = smoothstep(0.62, 0.82, d) * (1.0 - smoothstep(0.9, 1.0, d));
    if (ring < 0.01)
        discard;
    gl_FragColor = vec4(0.95, 0.97, 1.0, ring * 0.33); // soft cool-white, low alpha
}
