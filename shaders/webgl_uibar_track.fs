#version 100
precision mediump float;

// Meter background ("track"): a dark, semi-transparent slab the fill sits on.
// Colour is hardcoded (like webgl_decal) so the standalone-quad render path,
// which sets no colour uniform, can draw it as-is.
void main()
{
    gl_FragColor = vec4(0.14, 0.15, 0.19, 0.6);
}
