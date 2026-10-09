//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-08.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

// A corner of the unit square, -1 to 1: x is the side of the line the corner
// is on, and y is mapped to how far up the axis it is.
layout (location = 0) in vec2 a_position;

// binding 0 — per-frame. Only the prefix that is read is declared.
layout (std140) uniform PerFrame
{
    mat4 u_view;
    mat4 u_projection;
    vec4 u_camera_pos;       // xyz = world-space camera position, w = time
};

// binding 1 — per-material. Must be declared identically in both stages.
layout (std140) uniform MaterialBlock
{
    vec4 u_color;
    // x = the line's width as a fraction of a cell, y = cells per world unit,
    // z = the line's length.
    vec4 u_axis;
};

// The distance from the axis in cells, negative on one side of it.
out float v_distance;

// How much wider than the line the quad is on each side, as a fraction of
// the distance from the camera. It leaves room for the anti-aliasing and for
// a distant line, which is never drawn thinner than a pixel. This is a few
// pixels in any window of a reasonable size.
const float MARGIN = 0.01;

void main()
{
    // The line is a quad that contains the y axis and is turned around it to
    // face the camera. There is no PerDraw block as the node's transform is
    // deliberately ignored.
    vec2 to_camera = u_camera_pos.xz;
    vec3 side = dot(to_camera, to_camera) > 1e-12
                ? normalize(vec3(to_camera.y, 0.0, -to_camera.x))
                : vec3(1.0, 0.0, 0.0);

    vec3 center = vec3(0.0, (a_position.y * 0.5 + 0.5) * u_axis.z, 0.0);
    float depth = abs((u_view * vec4(center, 1.0)).z);
    float half_width = 0.5 * u_axis.x / u_axis.y + MARGIN * depth;

    // The distance is a linear function of the position in the quad's
    // plane, so interpolating it gives the exact distance in every fragment.
    v_distance = a_position.x * half_width * u_axis.y;

    gl_Position = u_projection * u_view
                  * vec4(center + side * (a_position.x * half_width), 1.0);
}
