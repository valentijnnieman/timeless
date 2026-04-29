#version 300 es
precision highp float;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D albedoMap;   // unit 0 — base colour texture
uniform sampler2D shadowMap;   // unit 1 — shadow depth map
uniform sampler2D normalMap;   // unit 2 — tangent-space normal map

uniform float metallic;
uniform float roughness;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;

#define MAX_POINT_LIGHTS 64
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

out vec4 fragColor;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float r)
{
    float a = r * r;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float r)
{
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float r)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, r) * GeometrySchlickGGX(NdotL, r);
}

float shadowFactor(vec4 fragPosLS, vec3 N, vec3 L)
{
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 ||
        proj.y < 0.0 || proj.y > 1.0 ||
        proj.z > 1.0)
        return 0.0;

    float currentDepth = proj.z;
    float bias = max(0.005 * (1.0 - dot(N, L)), 0.001);

    float shadow = 0.0;
    vec2 texelSize = vec2(1.0 / 2048.0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closest = texture(shadowMap,
                proj.xy + vec2(float(x), float(y)) * texelSize).r;
            shadow += (currentDepth - bias > closest) ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

// Compute a TBN matrix from screen-space derivatives — no tangent attribute needed.
mat3 computeTBN()
{
    vec3 dPdx = dFdx(FragPos);
    vec3 dPdy = dFdy(FragPos);
    vec2 dUVdx = dFdx(TexCoord);
    vec2 dUVdy = dFdy(TexCoord);

    float det = dUVdx.x * dUVdy.y - dUVdy.x * dUVdx.y;
    // Avoid division by zero on degenerate UVs
    if (abs(det) < 1e-6) det = sign(det) * 1e-6;

    vec3 T = (dPdx * dUVdy.y - dPdy * dUVdx.y) / det;
    vec3 Ngeom = normalize(Normal);
    T = normalize(T - dot(T, Ngeom) * Ngeom); // re-orthogonalise
    vec3 B = cross(Ngeom, T);
    return mat3(T, B, Ngeom);
}

void main()
{
    vec3 albedo = texture(albedoMap, TexCoord).rgb;

    // Decode normal map (tangent space) and transform to world space
    mat3 TBN = computeTBN();
    vec3 normalSample = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;
    vec3 N = normalize(TBN * normalSample);

    vec3 V = normalize(cameraPos - FragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 color = vec3(0.0);

    // Directional light
    {
        vec3 L = normalize(-lightPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 diffuse      = kD * albedo / PI;
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular     = numerator / denominator;

        float shadow = shadowFactor(FragPosLightSpace, N, L);
        color += (diffuse + specular) * lightColor * NdotL * (1.0 - shadow);
    }

    // Point lights
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        if (i >= numPointLights) break;

        vec3 L = normalize(pointLightPositions[i] - FragPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float dist        = length(pointLightPositions[i] - FragPos);
        float attenuation = 1.0 / (dist * dist);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 diffuse      = kD * albedo / PI;
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular     = numerator / denominator;

        color += (diffuse + specular) * pointLightColors[i] * attenuation * NdotL;
    }

    // Ambient
    color += ambientStrength * albedo;

    // Reinhard tonemapping + gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    fragColor = vec4(color, 1.0);
}
