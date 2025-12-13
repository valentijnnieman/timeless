#version 100
precision mediump float;
varying vec2 TexCoord;

uniform vec4 highlightColor;
uniform sampler2D texture1;

uniform float time;

float speed = 10.0;

void main()
{
  vec4 color = vec4(highlightColor.r, highlightColor.g, highlightColor.b, 1.0);
  gl_FragColor = vec4(color.rgb, texture2D(texture1, TexCoord).a * color.a);
}
