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
    vec3 color;
} fs_in;

void main()
{
    color = vec4(fs_in.color, 1.0);
}
