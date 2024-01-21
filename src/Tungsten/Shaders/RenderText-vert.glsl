//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 100

attribute highp vec2 a_Position;
attribute highp vec2 a_TextureCoord;

uniform highp mat4 u_MvpMatrix;

varying highp vec2 v_TextureCoord;

void main()
{
    gl_Position = u_MvpMatrix * vec4(a_Position, 0, 1);
    v_TextureCoord = a_TextureCoord;
}
