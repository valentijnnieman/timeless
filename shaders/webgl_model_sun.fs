#version 100
precision mediump float;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform vec3 materialDiffuse;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos); // Directional light from the sun

    // Correct view direction: from fragment to camera
    vec3 viewDir = normalize(cameraPos - FragPos);

    float edge = dot(norm, viewDir);

    // Ambient component
    vec3 ambient = ambientStrength * materialDiffuse;

    // Outline appears when edge is near 0 (perpendicular to view)
    float outlineWidth = 0.005; // Adjust for thickness
    float outlineFactor = smoothstep(0.0, outlineWidth, abs(edge));
    vec3 outlineColor = vec3(0.0, 0.0, 0.0); // Black outline

    // Diffuse component
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse * lightColor;

    // Combine
    vec3 result = ambient + diffuse;

    // Gamma correction
    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    gl_FragColor = vec4(gammaCorrected, 1.0);
}
