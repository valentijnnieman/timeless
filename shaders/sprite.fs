#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform int index;
uniform int xAmount;
uniform float frameWidth;

void main()
{
    float step = 1.0 / frameWidth;
    vec2 pos = vec2((step * (index * 2.0)) + (TexCoord.x / xAmount), TexCoord.y);
    // pos = fract(pos + vec2(step * 1.0, 0.0));
    FragColor = texture(texture1, pos);
}