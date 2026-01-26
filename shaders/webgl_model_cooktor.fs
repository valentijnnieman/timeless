#version 100
precision mediump float;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;

const float PI = 3.14159265359;

// Schlick's approximation for Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX / Trowbridge-Reitz normal distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

// Schlick-GGX geometry function (single term)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith's method for geometry function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 L = normalize(-lightPos); // Directional light
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    // float NDF = DistributionGGX(N, H, roughness);
    float NDF = 1.0; // For testing only
    float G   = GeometrySmith(N, V, L, roughness);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NdotL = max(dot(N, L), 0.0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.001;
    vec3 specular     = numerator / denominator;

    // kS is energy preserved for specular, kD for diffuse
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Lambertian diffuse
    vec3 diffuse = kD * albedo / PI;

    // Ambient
    vec3 ambient = ambientStrength * albedo;

    vec3 color = (diffuse + specular) * lightColor * NdotL + ambient;

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    gl_FragColor = vec4(color, 1.0);
    // gl_FragColor = vec4(vec3(NDF), 1.0);
    // gl_FragColor = vec4(vec3(denominator), 1.0);
}
