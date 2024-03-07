#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

float circle(in vec2 _st, in float _radius){
    vec2 dist = _st-vec2(0.0);
	return 1.-smoothstep(_radius-(_radius*0.01),
                         _radius+(_radius*0.01),
                         dot(dist,dist)*12.0);
}

void main()
{
    vec2 st = TexCoords.xy;
    vec2 mp = mousePosition.xy;

    vec2 f = vec2((st.x - mp.x) * 2.0, distance(st.y, (1.0 - mp.y)));

    vec3 color = vec3(circle(f,0.01));
    vec3 borderColor = vec3(1.0 - circle(f,0.011));

    FragColor = vec4(borderColor * vec3(texture(screenTexture, TexCoords)), 1.0 - color.z);
}  