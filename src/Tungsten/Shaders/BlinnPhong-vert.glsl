//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-23.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

#if defined(USE_DIFFUSE_MAP) || defined(USE_SPECULAR_MAP)
    #define USE_TEXTURE_COORD
#endif

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
#ifdef USE_TEXTURE_COORD
layout (location = 2) in vec2 a_tex_coord;
#endif

uniform mat4 u_model_view_matrix;
uniform mat4 u_proj_matrix;
uniform mat3 u_normal_matrix;

out VS_OUT
{
    vec3 frag_pos;
    vec3 view_pos;
    vec3 normal;
    #ifdef USE_TEXTURE_COORD
    vec2 texcoord;
    #endif
} vs_out;

void main()
{
    vs_out.frag_pos = vec3(u_model_view_matrix * vec4(a_position, 1.0));
    vs_out.view_pos = -vs_out.frag_pos;
    vs_out.normal = u_normal_matrix * a_normal;
    #ifdef USE_TEXTURE_COORD
    vs_out.texcoord = a_tex_coord;
    #endif
    gl_Position = u_proj_matrix * vec4(vs_out.frag_pos, 1.0);
}
