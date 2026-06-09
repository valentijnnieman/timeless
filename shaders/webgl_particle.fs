#version 100
precision mediump float;

uniform vec4 color;

varying vec2 vUV;

void main()
{
    // Soft round droplet: fade to transparent toward the quad edge.
    float d = length(vUV - vec2(0.5)) * 2.0; // 0 at centre, 1 at edge
    float a = color.a * smoothstep(1.0, 0.35, d);
    if (a < 0.01) discard;
    gl_FragColor = vec4(color.rgb, a);
}
