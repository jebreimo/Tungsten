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

in vec2 v_tex_position;
uniform sampler2D u_texture;
uniform vec3 u_color_delta;
out vec4 fragColor;

void main()
{
    vec4 tex_color = texture(u_texture, v_tex_position);
    vec3 result = max(tex_color.rgb + u_color_delta, vec3(0.0));
    fragColor = vec4(result, 1.0);
}
