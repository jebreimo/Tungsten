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

#include "Tungsten/ColorSpace.glsl"

in vec2 v_uv;
// The grid coordinate of the world's origin.
flat in vec2 v_origin;

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

out vec4 frag_color;

// The "pristine grid" from Ben Golus' article The Best Darn Grid Shader (Yet):
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8
//
// Returns how much of the pixel is covered by the grid lines of each axis: x
// is the lines where uv.x is a whole number and y the ones where uv.y is. The
// lines are one cell apart in uv and line_width is their width as a fraction
// of a cell.
vec2 pristine_grid(vec2 uv, vec2 line_width)
{
    line_width = clamp(line_width, 0.0, 1.0);

    // How much uv changes across the pixel, per axis.
    vec4 uv_ddxy = vec4(dFdx(uv), dFdy(uv));
    vec2 uv_deriv = vec2(length(uv_ddxy.xz), length(uv_ddxy.yw));

    // Lines wider than half a cell are drawn as the gaps between them and
    // inverted at the end, as the tricks below only work for thin lines.
    vec2 invert_line = vec2(greaterThan(line_width, vec2(0.5)));
    vec2 target_width = mix(line_width, 1.0 - line_width, invert_line);

    // A line is never drawn thinner than a pixel; it is faded instead, which
    // preserves its apparent brightness without aliasing.
    vec2 draw_width = min(max(target_width, uv_deriv), vec2(0.5));
    vec2 line_aa = max(uv_deriv, 0.000001) * 1.5;

    vec2 grid_uv = abs(fract(uv) * 2.0 - 1.0);
    grid_uv = mix(1.0 - grid_uv, grid_uv, invert_line);

    vec2 grid = 1.0 - smoothstep(draw_width - line_aa, draw_width + line_aa,
                                 grid_uv);
    grid *= clamp(target_width / max(draw_width, 0.000001), 0.0, 1.0);

    // Where the cells approach the size of a pixel the lines would produce
    // moiré, so the grid fades to its average coverage there.
    grid = mix(grid, target_width, clamp(uv_deriv * 2.0 - 1.0, 0.0, 1.0));
    return mix(grid, 1.0 - grid, invert_line);
}

void main()
{
    // The axes are the grid lines through the origin, so they are drawn by
    // giving the lines nearest to it their own colours and width. on_axis.x
    // is the line where uv.x is constant, i.e. the z axis.
    vec2 on_axis = vec2(lessThan(abs(v_uv - v_origin), vec2(0.5)));
    vec2 grid = pristine_grid(v_uv, mix(u_grid.xy, u_axis.xx, on_axis));
    vec4 x_lines = mix(u_line_color, u_z_axis_color, on_axis.x);
    vec4 z_lines = mix(u_line_color, u_x_axis_color, on_axis.y);

    // The colours are linear, as in every uniform block, which is the space
    // coverage has to be blended in. The result is encoded on the way out.
    // The lines of one direction are blended on top of the other's.
    vec4 color = mix(u_base_color, x_lines, grid.x * x_lines.a);
    color = mix(color, z_lines, grid.y * z_lines.a);
    frag_color = vec4(linear_to_srgb(color.rgb), color.a);
}
