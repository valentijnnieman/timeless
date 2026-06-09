#version 300 es
precision highp float;
// Flat, 2D-looking emissive sun: no view-dependent shading, so the sphere + rays
// read as a flat cut-out from any angle. The bright sun body glows a warm yellow
// (with a gentle global pulse), but dark meshes — e.g. the black sunglasses —
// are kept dark instead of being painted over, by blending on the mesh's own
// material colour (`albedo`, set per mesh from the Blender material). Pushed
// past 1.0 so the lit parts blow out like a light source.
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform float time;
uniform vec3 albedo; // per-mesh material colour from the model

out vec4 fragColor;

void main()
{
    float swell = 0.85 + 0.15 * sin(time * 2.0); // brightness pulse, in sync w/ rays
    vec3 sun = vec3(1.0, 0.82, 0.35) * swell * 1.6; // glowing sun body

    // Dark materials (the sunglasses are ~black) stay their own colour; bright
    // ones glow as the sun. Smooth threshold on the material's peak channel.
    float lum = max(albedo.r, max(albedo.g, albedo.b));
    vec3 col = mix(albedo, sun, smoothstep(0.05, 0.2, lum));

    fragColor = vec4(col, 1.0);
}
