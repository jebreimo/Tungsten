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

#include "Tungsten/ColorSpace.glsl"

// The distance from the axis in cells, negative on one side of it.
in float v_distance;

// binding 1 — per-material. Must be declared identically in both stages.
layout (std140) uniform MaterialBlock
{
    vec4 u_color;
    // x = the line's width as a fraction of a cell, y = cells per world unit,
    // z = the line's length.
    vec4 u_axis;
};

out vec4 frag_color;

void main()
{
    // A single line drawn the same way as the ones in grid-frag.glsl, so it
    // matches the axes in the grid: it is never drawn thinner than a pixel,
    // but faded instead.
    float deriv = length(vec2(dFdx(v_distance), dFdy(v_distance)));
    float target_width = clamp(u_axis.x, 0.0, 0.5);
    float draw_width = min(max(target_width, deriv), 0.5);
    float line_aa = max(deriv, 0.000001) * 1.5;

    float coverage = 1.0 - smoothstep(draw_width - line_aa,
                                      draw_width + line_aa,
                                      2.0 * abs(v_distance));
    coverage *= clamp(target_width / max(draw_width, 0.000001), 0.0, 1.0);
    if (coverage == 0.0)
        discard;

    frag_color = vec4(linear_to_srgb(u_color.rgb), u_color.a * coverage);
}
