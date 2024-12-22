//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

layout(location = 0) in vec2 a_position;

uniform mat3 u_mv_matrix;
uniform mat3 u_p_matrix;
uniform float u_z;
uniform float u_point_size;

void main()
{
    vec3 pos = u_mv_matrix * u_p_matrix * vec3(a_position, 1.0);
    gl_Position = vec4(pos.xy, u_z, 1.0);
    gl_PointSize = u_point_size;
}
