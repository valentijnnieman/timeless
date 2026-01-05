#version 100
precision mediump float;

varying vec3 Normal;
varying vec3 FragPos; // Pass from vertex shader: world-space position

uniform vec3 color;   // Solid color
uniform vec3 viewPos; // Camera position in world space

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    float rim = 1.0 - abs(dot(norm, viewDir));
    float outline = smoothstep(0.0, 0.1, rim);
    // float outline = step(0.3, rim);

    vec3 finalColor = mix(vec3(0.0), color, outline);
    // gl_FragColor = vec4(finalColor, 1.0);
    gl_FragColor = vec4(norm, 1.0);
}
