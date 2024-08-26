#version 330 core
#define PI 3.1415926538
#define SAMPLES 2
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

float noiseIntensity = 0.0005;

vec2 wave(vec2 co)
{
    return vec2(sin(140 * PI * co.x), sin(140 * PI * co.y));
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

vec2 rand2(vec2 co)
{
    return vec2(rand(co), rand(co));
}


void main()
{
    vec2 coords = TexCoords;
    //vec2 light = vec2(0.0, 1.0);

    //vec2 direction = normalize(coords - light);
    //float dist = distance(coords, light);

    //vec4 col = texture(screenTexture, coords);

    //col.r = step(0.25, col.r);
    //col.g = step(0.25, col.g);
    //col.b = step(0.25, col.b);

    //if(col.r > 0.0 && col.g > 0.0 && col.b > 0.0)
    //    discard;

    //float ray_dist = 0.0;
    //FragColor = vec4(1.0, 1.0, 1.0, 0.15);
    //for(int i = 0; i < SAMPLES; i++)
    //{
    //    vec2 tx = coords + direction * ray_dist;
    //    float dt = distance(tx, coords);
    //    vec4 col = texture(screenTexture, tx);
    //    if(col.r + col.g + col.b > 0.75)
    //    {
    //        FragColor = vec4(0.0, 0.0, 0.0, 0.25);
    //    }
    //    if(ray_dist > dt)
    //    {
    //        discard;
    //    }

    //    ray_dist = ray_dist + 0.1;
    //}

    //FragColor = vec4(0.0, 0.0, 0.0, 0.75);
    //float dc = dot(l, direction);
    //float d2 = dot(l, l) - dc * dc;

    //vec2 norm = vec2(coords.x, coords.y) * 2.0 - 1.0;

    vec4 col = texture(screenTexture, coords);

    col.r = step(0.1, col.r);
    col.g = step(0.1, col.g);
    col.b = step(0.1, col.b);

    //float t = time / 20.0;
    //vec2 noise = rand2(TexCoords + t) * noiseIntensity;
    //vec2 wavey = wave(TexCoords + t) * noiseIntensity;

    if(col.r > 0.0 && col.g > 0.0 && col.b > 0.0)
        FragColor = vec4(0.1, 0.1, 0.1, 0.5);
    else
        //FragColor = col;
        discard;

}  