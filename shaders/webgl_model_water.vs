#version 300 es
// Water surface vertex shader (beach) — the water STRETCHES along one direction
// instead of sliding rigidly. One edge is anchored (so the shoreline stays put
// and never opens a seam) and the displacement grows with distance along
// SWAY_DIR, so the far/open-sea edge reaches out and pulls back like a swell.
// The normal stays world-up so no part can tilt away from the sun, and there's
// no vertical motion so it can't clip the sand.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
// locations 3/4 (bone weights/ids) exist in the mesh VAO but are unused here.

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float time;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

// All in WORLD units. The stretch runs along SWAY_DIR:
//   ANCHOR : position (projected onto SWAY_DIR) of the fixed edge — keep this at
//            your shoreline so it doesn't move. Flip SWAY_DIR's sign to anchor
//            the other side.
//   REACH  : distance from ANCHOR to the far edge (where the stretch is full).
//   SWAY   : how far that far edge reaches out and back.
const vec2  SWAY_DIR   = vec2(1.0, 0.3);
const float SWAY       = 64.0;
const float SWAY_SPEED = 0.9;   // radians/sec
const float ANCHOR     = -460.0;
const float REACH      = 980.0;

void main()
{
    TexCoord = aTexCoord;

    vec4 world = model * vec4(aPos, 1.0);

    vec2 dir = normalize(SWAY_DIR);
    float proj = dot(dir, world.xy);
    // 0 at the anchored edge, ramping to 1 at the far edge.
    float t = clamp((proj - ANCHOR) / REACH, 0.0, 1.0);
    // 0 -> SWAY -> 0: the rest shape is the un-stretched plane; it bulges out and
    // settles back (starts at 0 so there's no pop on the first frame).
    float reach = (0.5 - 0.5 * cos(time * SWAY_SPEED)) * SWAY;
    world.xy += dir * (t * reach);

    Normal  = vec3(0.0, 0.0, 1.0); // flat, facing world-up
    FragPos = world.xyz;
    gl_Position = projection * view * world;
}
