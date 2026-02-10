#version 100
precision mediump float;
#define PI 3.1415926538
uniform sampler2D screenTexture;
uniform float width;
uniform float height;
uniform float time;

varying vec2 TexCoords;

vec2 wave(vec2 co)
{
    return vec2(sin(140.0 * PI * co.x), sin(140.0 * PI * co.y));
}

void main(void)
{
    float w = 1.0 / width;
    float h = 1.0 / height;

    // Sobel kernels
    float kernelX[9];
    float kernelY[9];
    kernelX[0] = -1.0; kernelY[0] = -1.0;
    kernelX[1] =  0.0; kernelY[1] = -2.0;
    kernelX[2] =  1.0; kernelY[2] = -1.0;
    kernelX[3] = -2.0; kernelY[3] =  0.0;
    kernelX[4] =  0.0; kernelY[4] =  0.0;
    kernelX[5] =  2.0; kernelY[5] =  0.0;
    kernelX[6] = -1.0; kernelY[6] =  1.0;
    kernelX[7] =  0.0; kernelY[7] =  2.0;
    kernelX[8] =  1.0; kernelY[8] =  1.0;

    vec2 offsets[9];
    offsets[0] = vec2(-w, -h);
    offsets[1] = vec2( 0.0, -h);
    offsets[2] = vec2( w, -h);
    offsets[3] = vec2(-w,  0.0);
    offsets[4] = vec2( 0.0,  0.0);
    offsets[5] = vec2( w,  0.0);
    offsets[6] = vec2(-w,  h);
    offsets[7] = vec2( 0.0,  h);
    offsets[8] = vec2( w,  h);

    float gx = 0.0;
    float gy = 0.0;

    for(int i = 0; i < 9; i++) {
        vec3 color = texture2D(screenTexture, TexCoords + offsets[i]).rgb;
        float lum = dot(color, vec3(0.299, 0.587, 0.114)); // Luminance
        gx += kernelX[i] * lum;
        gy += kernelY[i] * lum;
    }

     // float noise = fract(sin(time * PI) + sin(dot(TexCoords.xy ,vec2(12.9898,78.233))) * 43758.5453);
     // float thickness = 0.2 + 0.1 * noise; // randomize thickness
     float thickness = 0.25;

     float edge = length(vec2(gx, gy));
     vec3 origColor = texture2D(screenTexture, TexCoords).rgb;
     float isEdge = edge > thickness ? 0.0 : 1.0;

    // Use original color for edges, black elsewhere
    gl_FragColor = vec4(mix(origColor, vec3(0.0), 1.0 - isEdge), 1.0);
}
