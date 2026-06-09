#version 300 es
// Sun.fbx (a spiky ball + sunglasses) rendered as a camera-facing billboard:
// the whole model is re-oriented to a camera-aligned basis every frame, so it
// always faces the viewer and never spins. It "breathes" with a *uniform* scale
// pulse rather than per-vertex normal displacement — the latter pulls a
// hard-edged mesh's split vertices apart and reveals the polygon grid as gaps.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform float time;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;

    // World placement + (assumed uniform) scale, pulled straight from the model
    // matrix — its rotation is discarded so the billboard can't spin.
    vec3  worldPos = model[3].xyz;
    float scale    = length(model[0].xyz);

    // Sun.fbx is a flat disc facing +X; the rays radiate in the YZ plane around
    // the disc centre (measured below). The disc body sits at YZ-radius <= ~1.0,
    // then there's a clear gap, then the ray tips reach out to ~3.0. We extend
    // ONLY the rays, pushing them radially outward in YZ. Because the push
    // direction depends only on position (not per-face normals), coincident
    // hard-edge vertices move together — so the rays grow without opening the
    // facet gaps that normal-displacement caused.
    const vec2  kDiscCenter = vec2(-0.038, 0.942); // sun centre (y, z) in model space
    const float kRayExtend  = 0.55;                // max ray push, raw model units

    vec2  off  = aPos.yz - kDiscCenter;
    float rYZ  = length(off);
    vec2  rdir = (rYZ > 1e-4) ? off / rYZ : vec2(0.0);
    float rayF = smoothstep(1.1, 1.8, rYZ);        // 0 on the body, 1 on the rays
    float ang  = atan(off.y, off.x);               // per-ray phase (shared along a ray)
    float wave = sin(time * 2.5 + ang * 3.0);      // rays push out and pull back

    vec3 disp = aPos;
    disp.yz += rdir * (rayF * kRayExtend * wave);

    // The disc body keeps a gentle *uniform* breathe (gap-free); the rays ride it
    // on top of their extension.
    float breathe = 1.0 + 0.06 * sin(time * 2.0);
    vec3 local = disp * (scale * breathe);

    // Camera-aligned basis from the view matrix rows, in world space:
    //   camRight   = screen-right
    //   camUp      = screen-up
    //   camToward  = toward the camera (out of the screen)
    vec3 camRight  = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp     = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 camToward = vec3(view[0][2], view[1][2], view[2][2]);

    // Map the model's LOCAL axes onto that basis. The model's "front" should
    // point at the camera; in Blender that's +X, and Blender is Z-up, so:
    //   local X (front) -> toward the camera
    //   local Z (up)    -> screen-up
    //   local Y (side)  -> screen-right
    // To re-aim it, swap which line uses camToward / camUp / camRight (and negate
    // a term to flip that axis). This cyclic mapping keeps the basis right-handed
    // so winding/back-face culling is unaffected.
    vec3 world = worldPos + local.x * camToward + local.z * camUp + local.y * camRight;

    FragPos = world;
    Normal = normalize(aNormal.x * camToward + aNormal.z * camUp + aNormal.y * camRight);
    gl_Position = projection * view * vec4(world, 1.0);
}
