//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-30.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

layout (location = 0) out vec4 color;

in VS_OUT
{
    vec3 color;
} fs_in;

void main()
{
    color = vec4(fs_in.color, 1.0);
}
