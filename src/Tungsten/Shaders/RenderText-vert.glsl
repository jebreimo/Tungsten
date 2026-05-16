//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
    precision highp int;
    precision highp float;
#endif

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TextureCoord;

uniform mat3 u_MvpMatrix;

out vec2 v_TextureCoord;

void main()
{
    vec3 pos = u_MvpMatrix * vec3(a_Position, 1);
    gl_Position = vec4(pos.xy, 0, pos.z);
    v_TextureCoord = a_TextureCoord;
}
