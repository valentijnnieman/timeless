#version 100
precision mediump float;

varying vec2 vUV;

// Debug shade-disc decal: a clearly-visible translucent disc with a bright ring
// at the exact edge, so the circle (and its radius) reads on the sand. Colour is
// hardcoded so this needs no uniforms beyond the transform matrices.
void main()
{
    float d = length(vUV - vec2(0.5)) * 2.0; // 0 centre, 1 edge
    if (d > 1.0) discard;
    float ring = smoothstep(0.8, 0.97, d);
    float a = mix(0.45, 0.95, ring);
    gl_FragColor = vec4(0.1, 0.5, 1.0, a); // bold blue
}
