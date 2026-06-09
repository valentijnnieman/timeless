#version 100
precision mediump float;

// Cumulative customer-rating meter fill: green. Hardcoded colour (see
// webgl_decal); the value is the quad's width (Transform scale).
void main()
{
    gl_FragColor = vec4(0.40, 0.85, 0.47, 0.95);
}
