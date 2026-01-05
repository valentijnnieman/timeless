#version 100
precision mediump float;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(FragPos - lightPos);

    // Toon shading
    float intensity = max(dot(norm, lightDir), 0.0);
    float levels = 3.0;
    float toonIntensity = floor(intensity * levels) / (levels - 1.0);
    vec3 toonColor = materialDiffuse * toonIntensity;

    // Simple crosshatch
    float hatch = step(fract((FragPos.x + FragPos.y) * 10.0), 0.5);
    toonColor *= mix(1.0, 0.8, hatch);

    // Ambient component
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * toonColor;

    // Diffuse component
    float diff = max(dot(norm, lightDir), 1.0);
    vec3 diffuse = diff * materialDiffuse * lightColor;

    // Specular component
    float specularStrength = 0.5;
    vec3 viewDir = lightDir; // Assuming the viewer is at the light position for simplicity
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * materialSpecular * lightColor;

    vec3 result = ambient + diffuse + specular;

    vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
    gl_FragColor = vec4(gammaCorrected, 1.0);
}
