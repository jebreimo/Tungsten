//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlStateManagement.hpp"

#include <GL/glew.h>

#include "GlTypeConversion.hpp"
#include "Tungsten/IOglWrapper.hpp"

namespace Tungsten
{
    bool is_blend_enabled()
    {
        return get_ogl_wrapper().isEnabled(GL_BLEND);
    }

    void set_blend_enabled(bool enabled)
    {
        if (enabled)
            get_ogl_wrapper().enable(GL_BLEND);
        else
            get_ogl_wrapper().disable(GL_BLEND);
    }

    void set_blend_function(BlendFunction src, BlendFunction dst)
    {
        get_ogl_wrapper().blendFunc(to_ogl_blend_function(src),
                                    to_ogl_blend_function(dst));
    }

    void set_viewport(int x, int y, int width, int height)
    {
        get_ogl_wrapper().viewport(x, y, width, height);
    }
}
