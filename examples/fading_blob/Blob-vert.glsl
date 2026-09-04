//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

// Location 0 is where the fixed AttributeSemantic convention (§13 of
// docs/scene_graph_design.md) puts POSITION, for every shader and every VAO.
// The blob is flat in its node's xy plane, so the attribute is a vec2 and z is
// supplied here.
layout (location = 0) in vec2 a_position;

// binding 0 — per-frame. The renderer binds the whole PerFrame block (camera,
// ambient, lights, time); a block only has to agree with the prefix it
// declares, and an unlit shape reads the two matrices and stops.
layout (std140) uniform PerFrame
{
    mat4 u_view;
    mat4 u_projection;
};

// binding 2 — per-draw. The renderer binds model matrix followed by normal
// matrix; the blob has no normals, so only the leading mat4 is declared.
layout (std140) uniform PerDraw
{
    mat4 u_model;
};

void main()
{
    gl_Position = u_projection * u_view * u_model
                  * vec4(a_position, 0.0, 1.0);
}
