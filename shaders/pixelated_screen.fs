#version 330 core
#define PI 3.1415926538
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

vec2 u_resolution = vec2(1920.0, 1080.0);

void main() {
	// Calculate the square size in pixel units based on the mouse position
	float square_size = floor(2.0 + 30.0 * (100.0 / u_resolution.x));

	// Calculate the square center and corners
	vec2 center = square_size * floor(TexCoords * u_resolution / square_size) + square_size * vec2(0.5, 0.5);
	vec2 corner1 = center + square_size * vec2(-0.5, -0.5);
	vec2 corner2 = center + square_size * vec2(+0.5, -0.5);
	vec2 corner3 = center + square_size * vec2(+0.5, +0.5);
	vec2 corner4 = center + square_size * vec2(-0.5, +0.5);

	// Calculate the average pixel color
	vec3 pixel_color = 0.4 * texture(screenTexture, center / u_resolution).rgb;
	pixel_color += 0.15 * texture(screenTexture, corner1 / u_resolution).rgb;
	pixel_color += 0.15 * texture(screenTexture, corner2 / u_resolution).rgb;
	pixel_color += 0.15 * texture(screenTexture, corner3 / u_resolution).rgb;
	pixel_color += 0.15 * texture(screenTexture, corner4 / u_resolution).rgb;

	// Fragment shader output
	FragColor = vec4(pixel_color, 1.0);
}