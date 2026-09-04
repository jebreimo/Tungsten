//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

// Attribute locations follow the fixed AttributeSemantic convention (§13 of
// docs/scene_graph_design.md), which is why the texture coordinate is at 4 and
// not at 1. A glyph quad is flat in its node's xy plane, so the position is
// vec2 and z is supplied here.
layout (location = 0) in vec2 a_position;
layout (location = 4) in vec2 a_tex_coord;

// binding 0 — per-frame. The renderer uploads the full PerFrame block (camera,
// ambient, lights, time) and binds the whole buffer, but a block only has to
// agree with the *prefix* of that data it declares: unlit text reads the two
// matrices and stops. Their std140 offsets — 0 and 64 — are what must match.
layout (std140) uniform PerFrame
{
    mat4 u_view;
    mat4 u_projection;
};

// binding 2 — per-draw. The renderer binds a 112-byte range holding the model
// matrix followed by the normal matrix; text has no normals, so it declares
// only the leading mat4. Declaring less than is bound is fine — WebGL2 only
// rejects the reverse.
layout (std140) uniform PerDraw
{
    mat4 u_model;
};

out vec2 v_tex_coord;

void main()
{
    v_tex_coord = a_tex_coord;
    gl_Position = u_projection * u_view * u_model
                  * vec4(a_position, 0.0, 1.0);
}
