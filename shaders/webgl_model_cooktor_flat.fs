#version 300 es
precision highp float;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

// Same uniforms as cooktor — unused ones are silently ignored by the driver,
// so the rendering system can set them without any changes.
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;
uniform float alpha;
uniform sampler2D shadowMap;

#define MAX_POINT_LIGHTS 64
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

out vec4 fragColor;

void main()
{
    // === Pick one mode, comment out the others ===

    // --- Mode 1: pure flat (default) ---
    // Outputs the raw albedo colour, zero lighting influence.
    vec3 color = albedo;

    // --- Mode 2: subtle directional — ambient + diffuse only, no specular ---
    // Keeps a faint shape hint (2.5D feel) without the PBR highlight blob.
    // vec3 N = normalize(Normal);
    // vec3 L = normalize(-lightPos);
    // float NdotL = max(dot(N, L), 0.0);
    // vec3 color = albedo * (ambientStrength + NdotL * 0.5);

    // --- Mode 3: cel / toon — two hard light bands ---
    // Very cartoon-y; good for testing the silhouette read.
    // vec3 N = normalize(Normal);
    // vec3 L = normalize(-lightPos);
    // float NdotL = max(dot(N, L), 0.0);
    // float band = NdotL > 0.5 ? 1.0 : (NdotL > 0.15 ? 0.65 : 0.35);
    // vec3 color = albedo * band;

    color = pow(color, vec3(1.0 / 2.2));
    fragColor = vec4(color, alpha);
}
