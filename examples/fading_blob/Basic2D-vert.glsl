//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

layout(location = 0) in vec2 a_position;

uniform mat3 u_model_view_matrix;
uniform mat3 u_projection_matrix;
uniform float u_z;

void main()
{
    vec3 pos = u_projection_matrix * u_model_view_matrix * vec3(a_position, 1.0);
    gl_Position = vec4(vec2(pos), u_z, 1.0);
}
