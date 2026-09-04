//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp float;
#endif

// binding 1 — per-material. The blob's whole appearance is this one colour,
// uploaded once when the material is created and only rebound when the
// renderer switches material.
layout (std140) uniform MaterialBlock
{
    vec4 u_color;
};

out vec4 frag_color;

void main()
{
    frag_color = u_color;
}
