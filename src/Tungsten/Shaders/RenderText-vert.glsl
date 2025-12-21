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

in vec2 a_Position;
in vec2 a_TextureCoord;

uniform mat4 u_MvpMatrix;

out vec2 v_TextureCoord;

void main()
{
    gl_Position = u_MvpMatrix * vec4(a_Position, 0, 1);
    v_TextureCoord = a_TextureCoord;
}
