#version 100
precision mediump float;
varying vec2 TexCoord;
varying vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;

void main()
{
    float distance = length(lightPos - FragPos);
    // float distance = 0.5;
    float attenuation = 1.0 / (1.0 + distance * 0.5); // tweak 0.5 for falloff
    vec3 lightColor = vec3(1.0, 1.0, 1.0) * attenuation;
    vec4 texColor = texture2D(texture1, TexCoord);
    gl_FragColor = vec4(texColor.rgb * lightColor, texColor.a);
    // gl_FragColor = texture2D(texture1, TexCoord);
}
