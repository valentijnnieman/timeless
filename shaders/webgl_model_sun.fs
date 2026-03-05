#version 300 es
precision highp float;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 materialDiffuse;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 cameraPos;

#define MAX_POINT_LIGHTS 32
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

out vec4 FragColorOut;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos);

    vec3 ambient = ambientStrength * materialDiffuse;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse * lightColor;

    vec3 pointDiffuse = vec3(0.0);
    for (int i = 0; i < numPointLights; ++i) {
        vec3 toLight = pointLightPositions[i] - FragPos;
        float dist = length(toLight);
        float attenuation = dist < 256.0 ? 1.0 : 0.0;
        vec3 plDir = normalize(toLight);
        float plDiff = max(dot(norm, plDir), 0.0);
        pointDiffuse += plDiff * attenuation * materialDiffuse * pointLightColors[i];
    }

    vec3 result = ambient + diffuse + pointDiffuse;
    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    FragColorOut = vec4(gammaCorrected, 1.0);
}
