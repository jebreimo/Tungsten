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
        return get_ogl_wrapper().is_enabled(GL_BLEND);
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
        get_ogl_wrapper().blend_func(to_ogl_blend_function(src),
                                    to_ogl_blend_function(dst));
    }

    bool is_depth_test_enabled()
    {
        return get_ogl_wrapper().is_enabled(GL_DEPTH_TEST);
    }

    void set_depth_test_enabled(bool enabled)
    {
        if (enabled)
            get_ogl_wrapper().enable(GL_DEPTH_TEST);
        else
            get_ogl_wrapper().disable(GL_DEPTH_TEST);
    }

    bool is_face_culling_enabled()
    {
        return get_ogl_wrapper().is_enabled(GL_CULL_FACE);
    }

    void set_face_culling_enabled(bool enabled)
    {
        if (enabled)
            get_ogl_wrapper().enable(GL_CULL_FACE);
        else
            get_ogl_wrapper().disable(GL_CULL_FACE);
    }

    void set_face_culling_mode(FaceCullingMode mode)
    {
        get_ogl_wrapper().cull_face(to_ogl_cull_mode(mode));
    }

    void set_viewport(int x, int y, int width, int height)
    {
        get_ogl_wrapper().viewport(x, y, width, height);
    }
}
