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
uniform vec3 u_diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 u_specular_albedo = vec3(0.7);
uniform float u_specular_power = 128.0;
uniform vec3 u_ambient = vec3(0.1, 0.1, 0.1);

out VS_OUT
{
    vec3 color;
} vs_out;

void main()
{
    vec4 p = u_mv_matrix * vec4(a_position, 1.0);
    vec3 n = normalize(mat3(u_mv_matrix) * a_normal);
    vec3 l = normalize(u_light_pos - p.xyz);
    vec3 v = normalize(-p.xyz);

    vec3 r = reflect(-l, n);
    vec3 diffuse = max(dot(n, l), 0.0) * u_diffuse_albedo;
    vec3 specular = pow(max(dot(r, v), 0.0), u_specular_power)
                    * u_specular_albedo;

    vs_out.color = u_ambient + diffuse + specular;
    gl_Position = u_proj_matrix * p;
}
