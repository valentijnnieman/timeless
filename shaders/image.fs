#version 330 core
out vec4 FragColor;
  
in vec3 color;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 highlightColor;

void main()
{
    FragColor = highlightColor * texture(texture1, vec2(TexCoord.x, TexCoord.y));
}