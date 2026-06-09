#version 100
precision mediump float;

// Live-satisfaction meter fill: warm gold. Hardcoded colour (see webgl_decal):
// the bar's *value* is encoded by the quad's width (its Transform scale), not by
// any colour uniform.
void main()
{
    gl_FragColor = vec4(1.0, 0.78, 0.30, 0.95);
}
