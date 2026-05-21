#version 100
precision mediump float;

uniform sampler2D screenTexture;

varying vec2 TexCoords;

void main()
{
    // Transparent variant of webgl_screen.fs. The offscreen FBO is cleared to
    // (0,0,0,0) and the scene is rendered with separate-alpha blending, so the
    // texture holds PREMULTIPLIED colour (rgb already multiplied by coverage).
    // We pass it straight through and let the composite use a premultiplied
    // "over" blend (GL_ONE, GL_ONE_MINUS_SRC_ALPHA), which lets the page's
    // background show through wherever nothing was drawn — without the white
    // halo that the opaque variant works around.
    gl_FragColor = texture2D(screenTexture, TexCoords);
}
