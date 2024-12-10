#version 330 core
#define CRT_CASE_BORDR 0.0125
#define SCAN_LINE_MULT 1250.0

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 lightPosition;
uniform vec2 mousePosition;

void main() {
	vec2 tc = vec2(TexCoords.x, TexCoords.y);

	// Distance from the center
	float dx = abs(0.5-tc.x);
	float dy = abs(0.5-tc.y);

	// Square it to smooth the edges
	dx *= dx;
	dy *= dy;

	tc.x -= 0.5;
	tc.x *= 1.0 + (dy * 0.25);
	tc.x += 0.5;

	tc.y -= 0.5;
	tc.y *= 1.0 + (dx * 0.5);
	tc.y += 0.5;

	// Get texel, and add in scanline if need be
	vec4 cta = texture2D(screenTexture, vec2(tc.x, tc.y));

	cta.rgb += sin(tc.y * SCAN_LINE_MULT) * 0.02;

	// Cutoff
	if(tc.y > 1.0 || tc.x < 0.0 || tc.x > 1.0 || tc.y < 0.0)
		cta = vec4(0.0);

	// Apply
	FragColor = cta;
}
