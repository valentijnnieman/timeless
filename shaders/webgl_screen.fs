#version 100
precision mediump float;
  
uniform sampler2D screenTexture;

varying vec2 TexCoords;

void main()
{
    // gl_FragColor = vec4(vec3(texture2D(screenTexture, TexCoords)), 1.0);
    vec4 texColor = texture2D(screenTexture, TexCoords);
    gl_FragColor = texColor;
}  
