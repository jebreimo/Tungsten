//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-11-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

uniform vec4 u_color = vec4(1.0, 1.0, 1.0, 1.0);
out vec4 color;

void main()
{
    color = u_color;
}
