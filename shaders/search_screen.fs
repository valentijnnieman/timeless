#version 330 core
#define PI 3.1415926538
#define SCAN_LINE_MULT 1250.0
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;
uniform vec4 SCREEN_COLOR;

const float range = 0.0009;
const float noiseQuality = 10.0;
const float noiseIntensity = 0.0005;
const float offsetIntensity = 0.01;
const float colorOffsetIntensity = 0.25;

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

float circle(in vec2 _st, in float _radius){
    vec2 dist = _st-vec2(0.0);
	return 1.-smoothstep(_radius-(_radius*0.01),
                         _radius+(_radius*0.01),
                         dot(dist,dist)*12.0);
}

void main()
{
    vec2 uv = TexCoords.xy;
    vec2 st = TexCoords.xy;
    vec2 tc = TexCoords.xy;
    vec2 mp = mousePosition.xy;

    // Distance from the center
    float dx = abs(0.5-tc.x);
    float dy = abs(0.5-tc.y);

    // Square it to smooth the edges
    dx *= dx;
    dy *= dy;

    tc.x -= 0.5;
    tc.x *= 1.0 + (dy * 0.05);
    tc.x += 0.5;

    tc.y -= 0.5;
    tc.y *= 1.0 + (dx * 0.05);
    tc.y += 0.5;

    vec2 f = vec2((st.x - mp.x) * 2.0, distance(st.y, (1.0 - mp.y)));

    vec3 color = vec3(circle(f,0.01));
    vec3 borderColor = vec3(1.0 - circle(f,0.011));

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

  vec2 offsetR = vec2(0.003 * sin(time), 0.0) * colorOffsetIntensity;
  vec2 offsetG = vec2(0.003 * (cos(time * 0.5)), 0.0) * colorOffsetIntensity;
  
  float r = texture(screenTexture, uv + offsetR).r;
  float g = texture(screenTexture, uv + offsetG).g;
  float b = texture(screenTexture, uv).b;

    vec4 tex = texture(screenTexture, tc);
  tex.rgb += sin(tc.y * SCAN_LINE_MULT) * 0.02;
  vec4 tex2 = vec4(r, g, b, 1.0);

  vec4 out_tex = mix(tex, tex2, 0.5);

    FragColor = vec4(borderColor * vec3(out_tex), 1.0 - color.z);
}  
