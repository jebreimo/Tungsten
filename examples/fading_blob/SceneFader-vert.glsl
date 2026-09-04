//****************************************************************************
// Copyright © 2024 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2024-12-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

in vec2 a_position;
in vec2 a_tex_position;
out vec2 v_tex_position;
void main()
{
    v_tex_position = a_tex_position;
    gl_Position = vec4(a_position, 0.0, 1.0);
}
