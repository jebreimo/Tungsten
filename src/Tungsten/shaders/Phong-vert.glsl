//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 410

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_mv_matrix;
uniform mat4 u_proj_matrix;

uniform vec3 u_light_pos = vec3(-100.0, -100.0, 100.0);

out VS_OUT
{
    vec3 normal;
    vec3 light;
    vec3 view;
} vs_out;

void main()
{
    vec4 p = u_mv_matrix * vec4(a_position, 1.0);
    vs_out.normal = mat3(u_mv_matrix) * a_normal;
    vs_out.light = u_light_pos - p.xyz;
    vs_out.view = -p.xyz;
    gl_Position = u_proj_matrix * p;
}
