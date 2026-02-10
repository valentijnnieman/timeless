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

#define MAX_POINT_LIGHTS 64
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform int numPointLights;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightPos); // Directional light from the sun

    vec3 ambient = ambientStrength * materialDiffuse;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialDiffuse * lightColor;

    // Point lights
    vec3 pointDiffuse = vec3(0.0);
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        if(i >= numPointLights) break;
        vec3 plDir = normalize(pointLightPositions[i] - FragPos);
        float plDiff = max(dot(norm, plDir), 0.0);
        pointDiffuse += plDiff * materialDiffuse * pointLightColors[i];
    }

    vec3 result = ambient + diffuse + pointDiffuse;
    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    gl_FragColor = vec4(gammaCorrected, 1.0);
}
