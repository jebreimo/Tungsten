//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************

// The encode every fragment shader owes the framebuffer.
//
// Colour textures are decoded to linear by the sampler and colour constants
// arrive linear in their uniform blocks, so all shading arithmetic here is
// linear. The default framebuffer holds sRGB, so the result has to be encoded
// on the way out. This is done in the shader rather than with an sRGB
// framebuffer because GLES 3.0 and WebGL2 have no GL_FRAMEBUFFER_SRGB to
// switch on; the cost is that blending then happens on encoded values, which
// an offscreen linear target would fix.
//
// The exact piecewise curve, matching Tungsten::linear_to_srgb on the CPU
// side and the hardware decode applied to sRGB textures.
vec3 linear_to_srgb(vec3 color)
{
    vec3 cutoff = vec3(lessThanEqual(color, vec3(0.0031308)));
    vec3 low = color * 12.92;
    vec3 high = 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;
    return mix(high, low, cutoff);
}
