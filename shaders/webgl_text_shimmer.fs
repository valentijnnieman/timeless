#version 100
precision mediump float;

uniform sampler2D text;
uniform vec4 textColor;
uniform float time;

varying vec2 TexCoords;

void main()
{
    float alpha = texture2D(text, TexCoords).r;

    // Diagonal shimmer band sweeping left-to-right across the logo.
    float pos = gl_FragCoord.x * 0.75 + gl_FragCoord.y * 0.25;

    // Repeats every 400px, sweeps at 220 px/sec (~2.8s per cycle).
    float t = mod(pos - time * 220.0, 400.0);

    // Wide soft band (~160px) so multiple letters are lit at once.
    float shimmer = max(0.0, 1.0 - abs(t - 160.0) / 160.0);
    shimmer = shimmer * shimmer; // sharpen the falloff

    // Shimmer to pure white; base (black) stays invisible between passes.
    vec3 final_color = mix(textColor.rgb, vec3(1.0, 1.0, 1.0), shimmer);

    gl_FragColor = vec4(final_color, textColor.a * alpha);
}
