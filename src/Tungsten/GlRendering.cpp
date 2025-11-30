//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include <Tungsten/GlRendering.hpp>
#include <Tungsten/IOglWrapper.hpp>

namespace Tungsten
{
    void set_clear_color(float r, float g, float b, float a)
    {
        get_ogl_wrapper().clear_color(r, g, b, a);
    }

    void set_clear_color(const Xyz::Vector4F& color)
    {
        set_clear_color(color[0], color[1], color[2], color[3]);
    }

    void set_clear_depth(float depth)
    {
        get_ogl_wrapper().clear_depth(depth);
    }

    void set_clear_stencil(int32_t stencil)
    {
        get_ogl_wrapper().clear_stencil(stencil);
    }

    void clear(ClearBufferMask mask)
    {
        get_ogl_wrapper().clear(static_cast<GLenum>(mask));
    }

    void finish_rendering()
    {
        get_ogl_wrapper().finish();
    }
}
