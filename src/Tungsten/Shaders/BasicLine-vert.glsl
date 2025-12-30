//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-30.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

layout (location = 0) in vec3 a_position;
layout (location = 0) in vec3 a_normal;

uniform mat4 u_mv_matrix;
uniform mat4 u_proj_matrix;

uniform vec3 u_light_pos = vec3(-100.0, -100.0, 100.0);
uniform vec3 u_diffuse_albedo = vec3(1.0, 0.0, 0.0);
uniform vec3 u_ambient = vec3(0.6, 0.6, 0.6);

out VS_OUT
{
    vec3 color;
} vs_out;

void main()
{
    vec4 p = u_mv_matrix * vec4(a_position, 1.0);
    vec3 n = normalize(mat3(u_mv_matrix) * a_normal);
    vec3 l = normalize(u_light_pos - p.xyz);
    vec3 diffuse = max(dot(n, l), 0.0) * u_diffuse_albedo;

    vs_out.color = u_ambient + diffuse;
    gl_Position = u_proj_matrix * p;
}
