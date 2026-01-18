//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
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
    #ifdef USE_TEXTURES
    vec2 texcoord;
    #endif
} fs_in;

uniform sampler2D u_texture;

void main()
{
    vec3 rgb_color = fs_in.color;
    #ifdef USE_TEXTURES
    rgb_color *= texture(u_texture, fs_in.texcoord).rgb;
    #endif
    color = vec4(rgb_color, 1.0);
}
