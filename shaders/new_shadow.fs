#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform float size;
uniform float shadowValue;
uniform vec2 lightPosition;
uniform vec2 mousePosition;
uniform vec4 SCREEN_COLOR;

void main()
{
    vec4 col = texture(screenTexture, TexCoords);

    FragColor = col;

    float p = shadowValue / size;
    bool shadow = true;
    vec4 light = vec4(0.0);

    if(col.r == SCREEN_COLOR.r && col.g == SCREEN_COLOR.g && col.b == SCREEN_COLOR.b)
    {
        for(int i = 0; i < 6; i++)
        {
            float dist = float(i) * p;

            float uvx = TexCoords.x - (dist * 0.1);
            float uvy = TexCoords.y - dist;

            vec4 next = texture(screenTexture, vec2(clamp(uvx, 0.0, 1.0), clamp(uvy, 0.0, 1.0)));
            //if(next.r > 0.9 && next.g > 0.9 && next.b > 0.9)
            //if(next.z > 0.9)
            if(next.r != SCREEN_COLOR.r && next.g != SCREEN_COLOR.g && next.b != SCREEN_COLOR.b)
            {
                //FragColor = vec4(0.0, 0.0, 0.0, 0.5);
                light = vec4(0.9, 0.4, 0.1, 0.4);
                shadow = false;
            }
        }
    }

    //if(col.r > 0.1 && col.g > 0.1 && col.b > 0.1)
    //if(col.z > 0.9)
    if(shadow)
    {
        discard;
        //FragColor = col;
    }
    FragColor = light;
}  
