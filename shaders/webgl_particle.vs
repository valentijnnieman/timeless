#version 100
attribute vec3 aPos;       // quad local, -1..1 in XY
attribute vec3 aColor;     // unused
attribute vec2 aTexCoord;  // 0..1

uniform mat4 projection;
uniform mat4 model;        // translate(world pos) * scale(size), per particle
uniform mat4 view;

varying vec2 vUV;

void main()
{
    // Camera-facing billboard: take the particle's world position + size from
    // the model matrix and orient the quad to the camera using the view's
    // right/up axes, so round droplets always face the camera.
    vec3 center = vec3(model[3]);
    float size  = length(vec3(model[0]));

    vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 world = center + (aPos.x * camRight + aPos.y * camUp) * size;
    gl_Position = projection * view * vec4(world, 1.0);
    vUV = aTexCoord;
}
