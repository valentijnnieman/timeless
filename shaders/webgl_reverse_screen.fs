#version 100
precision mediump float;
#define SCAN_LINE_MULT 1250.0

varying vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

const float range = 0.0025;
const float noiseQuality = 10.0;
const float noiseIntensity = 0.0024;
const float offsetIntensity = 0.1;
const float colorOffsetIntensity = 1.3;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float verticalBar(float pos, float uvY, float offset)
{
    float edge0 = (pos - range);
    float edge1 = (pos + range);

    float x = smoothstep(edge0, pos, uvY) * offset;
    x -= smoothstep(pos, edge1, uvY) * offset;
    return x;
}

void main()
{
	vec2 uv = TexCoords.xy;
  vec2 tc = TexCoords.xy;

  // Distance from the center
  float dx = abs(0.5-tc.x);
  float dy = abs(0.5-tc.y);

  // Square it to smooth the edges
  dx *= dx;
  dy *= dy;

  tc.x -= 0.5;
  tc.x *= 1.0 + (dy * 0.15);
  tc.x += 0.5;

  tc.y -= 0.5;
  tc.y *= 1.0 + (dx * 0.15);
  tc.y += 0.5;
  
  for (float i = 0.0; i < 0.71; i += 0.1313)
  {
      float d = mod(time * i, 1.7);
      float o = sin(1.0 - tan(time * 0.24 * i));
      o *= offsetIntensity;
      uv.x += verticalBar(d, uv.y, o);
  }
  
  float uvY = uv.y;
  uvY *= noiseQuality;
  uvY = float(int(uvY)) * (1.0 / noiseQuality);
  float noise = rand(vec2(time * 0.00001, uvY));
  uv.x += noise * noiseIntensity;

  vec2 offsetR = vec2(0.006 * sin(time), 0.0) * colorOffsetIntensity;
  vec2 offsetG = vec2(0.0073 * (cos(time * 0.97)), 0.0) * colorOffsetIntensity;
  
  float r = texture2D(screenTexture, uv + offsetR).r;
  float g = texture2D(screenTexture, uv + offsetG).g;
  float b = texture2D(screenTexture, uv).b;

  vec4 tex = texture2D(screenTexture, tc);
  tex.rgb += sin(tc.y * SCAN_LINE_MULT) * 0.02;
  vec4 tex2 = vec4(r, g, b, 1.0);
  gl_FragColor = mix(tex, tex2, 0.5);
}
