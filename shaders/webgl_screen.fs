#version 100
precision mediump float;
  
uniform sampler2D screenTexture;

varying vec2 TexCoords;

void main()
{
    // Force opaque output: the offscreen FBO's alpha channel gets eroded along
    // anti-aliased text edges (text is blended with SRC_ALPHA over an opaque
    // white clear), so passing that alpha through to the screen composite blends
    // the white clear color back in as a halo/outline around dark glyphs.
    vec4 texColor = texture2D(screenTexture, TexCoords);
    gl_FragColor = vec4(texColor.rgb, 1.0);
}  
