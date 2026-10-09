//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

// A corner of the unit square, -1 to 1. It is not a position in the scene:
// the square is stretched over the world's xz plane below.
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
    vec4 u_line_color;
    vec4 u_base_color;
    vec4 u_x_axis_color;
    vec4 u_z_axis_color;
    // x, y = line widths as fractions of a cell, z = cells per world unit,
    // w = half the side of the square the grid is drawn on.
    vec4 u_grid;
    // x = the width of the axis lines as a fraction of a cell.
    vec4 u_axis;
};

out vec2 v_uv;
// The grid coordinate of the world's origin.
flat out vec2 v_origin;

void main()
{
    // The grid is infinite because the square follows the camera: it is
    // always centred below it and reaches past the far plane. There is no
    // PerDraw block as the node's transform is deliberately ignored.
    vec2 offset = a_position * u_grid.w;
    vec2 camera = u_camera_pos.xz;

    // The grid coordinate is the world position relative to the cell the
    // camera is in, i.e. world * scale - floor(camera * scale), which keeps
    // it small and therefore precise far from the world's origin.
    v_uv = offset * u_grid.z + fract(camera * u_grid.z);
    v_origin = -floor(camera * u_grid.z);

    gl_Position = u_projection * u_view
                  * vec4(camera.x + offset.x, 0.0, camera.y + offset.y, 1.0);
}
