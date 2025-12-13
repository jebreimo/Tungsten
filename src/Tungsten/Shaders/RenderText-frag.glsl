//****************************************************************************
// Copyright © 2021 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2021-10-09.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 100

#ifdef GL_ES
    precision highp float;
#endif

varying vec2 v_TextureCoord;

uniform sampler2D u_Texture;
uniform vec4 u_TextColor;

void main()
{
    vec4 texCol = texture2D(u_Texture, v_TextureCoord);
    float value = max(texCol.r, max(texCol.g, texCol.b));
    if (value == 0.0)
        discard;
    gl_FragColor = vec4(u_TextColor.r,
                        u_TextColor.g,
                        u_TextColor.b,
                        u_TextColor.a * value);
}
