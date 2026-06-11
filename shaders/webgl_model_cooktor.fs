#version 300 es
precision highp float;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform vec3 lightPos;         // directional light: direction vector pointing toward the light
uniform vec3 lightColor;       // directional light color/intensity
uniform float ambientStrength;
uniform vec3 cameraPos;
uniform sampler2D shadowMap;

uniform float burnAmount;      // 0..1 sun-burn intensity for this entity (0 = none)
uniform vec3  burnColor;       // albedo is multiplied toward this at full burn
uniform float tanAmount;       // 0..1 sun-tan intensity for this entity (0 = none)
uniform vec3  tanColor;        // albedo is multiplied toward this at full tan
uniform float sheen;           // 0..1 freshly-oiled look (glossier + a bright rim)
uniform float modelAlpha;      // 0..1 fragment alpha for translucent models (1 = opaque)
uniform float isPants;         // 1.0 while drawing the bod's "Pants" mesh, else 0.0
uniform vec3  pantsColor;      // per-bod swim-trunk tint (white = unchanged)

#define MAX_POINT_LIGHTS 64
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

out vec4 fragColor;

const float PI = 3.14159265359;

// Schlick's approximation for Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX / Trowbridge-Reitz normal distribution function
float DistributionGGX(vec3 N, vec3 H, float r)
{
    float a = r * r;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// Schlick-GGX geometry function (single term)
float GeometrySchlickGGX(float NdotV, float r)
{
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith's method for geometry function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float r)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, r) * GeometrySchlickGGX(NdotL, r);
}

// 3x3 PCF shadow with slope-scaled bias
float shadowFactor(vec4 fragPosLS, vec3 N, vec3 L)
{
    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;

    // Outside the light frustum — lit
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

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - FragPos);

    // Sun exposure for THIS fragment: lit (out of shadow) AND facing the sun.
    // Sun-burn reddens only exposed skin — fragments in the umbrella's (or self)
    // shadow keep their normal albedo, so a bod half in shade burns only on the
    // sunlit half. This reuses the existing directional shadow sample.
    vec3  Lsun        = normalize(-lightPos);
    float sunNdotL    = max(dot(N, Lsun), 0.0);
    float sunShadow   = shadowFactor(FragPosLightSpace, N, Lsun);
    float sunExposure = (1.0 - sunShadow) * sunNdotL;
    // Per-bod swim-trunk colour: tint only the "Pants" mesh (isPants==1) by the
    // entity's pantsColor; every other mesh keeps its own albedo.
    vec3  baseAlbedo  = albedo * mix(vec3(1.0), pantsColor, isPants);
    // Tan browns the skin and burn reddens it; both only on sunlit fragments and
    // they stack (a bod that tanned then started burning reads reddish-brown).
    vec3  albedoB     = baseAlbedo
                        * mix(vec3(1.0), tanColor,
                              clamp(tanAmount * sunExposure, 0.0, 1.0))
                        * mix(vec3(1.0), burnColor,
                              clamp(burnAmount * sunExposure, 0.0, 1.0));

    // Freshly-oiled "sheen": glossier (lower roughness) so the sun highlight
    // pops, plus a bright Fresnel rim — used as the "this bod is being lotioned"
    // selection cue.
    float sheenC = clamp(sheen, 0.0, 1.0);
    float rough  = mix(roughness, 0.05, sheenC);

    vec3 F0 = mix(vec3(0.04), albedoB, metallic);

    vec3 color = vec3(0.0);

    // Directional light (sun) — no attenuation, lightPos is a direction vector
    {
        vec3 L = Lsun;
        vec3 H = normalize(V + L);
        float NdotL = sunNdotL;

        float NDF = DistributionGGX(N, H, rough);
        float G   = GeometrySmith(N, V, L, rough);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 diffuse      = kD * albedoB / PI;
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular     = numerator / denominator;

        color += (diffuse + specular) * lightColor * NdotL * (1.0 - sunShadow);
    }

    // Point lights — with inverse-square attenuation (no shadow)
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        if (i >= numPointLights) break;

        vec3 L = normalize(pointLightPositions[i] - FragPos);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);

        float NDF = DistributionGGX(N, H, rough);
        float G   = GeometrySmith(N, V, L, rough);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float dist        = length(pointLightPositions[i] - FragPos);
        float attenuation = 1.0 / (dist * dist);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 diffuse      = kD * albedoB / PI;
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
        vec3 specular     = numerator / denominator;

        color += (diffuse + specular) * pointLightColors[i] * attenuation * NdotL;
    }

    // Ambient
    vec3 ambient = ambientStrength * albedoB;
    color += ambient;

    // Sheen selection rim: a soft bright edge so a lotioned bod stands out.
    color += sheenC * pow(1.0 - max(dot(N, V), 0.0), 3.0) * vec3(0.7);

    // Reinhard tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    fragColor = vec4(color, modelAlpha);
}
