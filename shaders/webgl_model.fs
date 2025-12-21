#version 100
precision mediump float;

varying vec2 TexCoord;
varying vec3 Normal;

uniform sampler2D texture1;

void main()
{
    // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    gl_FragColor = texture2D(texture1, TexCoord);
    // gl_FragColor = vec4(TexCoord, 0.0, 1.0);
}
