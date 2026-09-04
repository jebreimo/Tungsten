//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-08-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#version 300 es

#ifdef GL_ES
precision highp float;
#endif

in vec2 v_tex_coord;

// Sampler 0 of the family's sampler list, so it samples texture unit 0 — the
// unit the renderer binds a material's first texture to.
uniform sampler2D u_text_atlas;

// binding 1 — per-material. The whole appearance of text is this one colour:
// TextSystem resolves a material per (atlas, colour) pair, so items sharing a
// style share this buffer and the renderer binds it once for all of them.
layout (std140) uniform MaterialBlock
{
    vec4 u_text_color;
};

out vec4 frag_color;

void main()
{
    // The atlas stores coverage in its colour channels rather than in alpha,
    // so the brightest channel is the glyph's coverage at this texel.
    vec4 texel = texture(u_text_atlas, v_tex_coord);
    float coverage = max(texel.r, max(texel.g, texel.b));
    // Fully uncovered texels are discarded rather than blended: at one quad
    // per glyph most of a text's fragments fall between the letters.
    if (coverage == 0.0)
        discard;
    frag_color = vec4(u_text_color.rgb, u_text_color.a * coverage);
}
