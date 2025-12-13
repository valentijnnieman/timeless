#version 100
precision mediump float;
varying vec2 TexCoord;

uniform sampler2D texture1;
uniform float time;

float speed = 20.0;
void main()
{
    float red = 0.5 + 0.5 * sin(time * speed);
    float green = 0.5 + 0.5 * sin(time * speed);
    float blue = 0.5 + 0.5 * sin(time * speed);
    vec4 color = vec4(red, green, blue, 1.0);
    gl_FragColor = color * texture2D(texture1, TexCoord);
}
