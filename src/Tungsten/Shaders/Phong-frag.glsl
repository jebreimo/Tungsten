//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 410

layout (location = 0) out vec4 color;

in VS_OUT
{
    vec3 normal;
    vec3 light;
    vec3 view;
} fs_in;

uniform vec3 u_diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 u_specular_albedo = vec3(0.7);
uniform float u_specular_power = 128.0;

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 light = normalize(fs_in.light);
    vec3 view = normalize(fs_in.view);

    vec3 ref = reflect(-light, normal);

    vec3 diffuse = max(dot(normal, light), 0.0) * u_diffuse_albedo;
    vec3 specular = pow(max(dot(ref, view), 0.0), u_specular_power)
                    * u_specular_albedo;
    color = vec4(diffuse + specular, 1.0);
}
