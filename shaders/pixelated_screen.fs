#version 330 core
out vec4 fragColor;
uniform sampler2D screenTexture;
in vec2 TexCoords;

uniform vec3 palette[8];
uniform int paletteSize;

const int indexMatrix4x4[16] = int[](0,  8,  2,  10,
                                     12, 4,  14, 6,
                                     3,  11, 1,  9,
                                     15, 7,  13, 5);

float indexValue() {
    int x = int(mod(gl_FragCoord.x, 4));
    int y = int(mod(gl_FragCoord.y, 4));
    return indexMatrix4x4[(x + y * 4)] / 16.0;
}

float hueDistance(float h1, float h2) {
    float diff = abs((h1 - h2));
    return min(abs((1.0 - diff)), diff);
}

vec3[2] closestColors(float hue) {
    vec3 ret[2];
    vec3 closest = vec3(-2, 0, 0);
    vec3 secondClosest = vec3(-2, 0, 0);
    vec3 temp;
    for (int i = 0; i < paletteSize; ++i) {
        temp = palette[i];
        float tempDistance = hueDistance(temp.x, hue);
        if (tempDistance < hueDistance(closest.x, hue)) {
            secondClosest = closest;
            closest = temp;
        } else {
            if (tempDistance < hueDistance(secondClosest.x, hue)) {
                secondClosest = temp;
            }
        }
    }
    ret[0] = closest;
    ret[1] = secondClosest;
    return ret;
}

vec3 dither(vec3 color) {
    vec3 hsl = rgbToHsl(color);
    vec3 colors[2] = closestColors(hsl.x);
    vec3 closestColor = cs[0];
    vec3 secondClosestColor = cs[1];
    float d = indexValue();
    float hueDiff = hueDistance(hsl.x, closestColor.x) /
                    hueDistance(secondClosestColor.x, closestColor.x);
    return hslToRgb(hueDiff < d ? c1 : c2);
}

void main () {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    fragColor = vec4(dither(color), 1);
}
