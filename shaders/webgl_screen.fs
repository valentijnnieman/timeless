#version 100
precision mediump float;
  
uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

varying vec2 TexCoords;

void main()
{
    gl_FragColor = vec4(vec3(texture2D(screenTexture, TexCoords)), 1.0);
}  
