//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-11-30.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlStateManagement.hpp"

#include <GL/glew.h>
#include "Tungsten/Gl/IOglWrapper.hpp"
#include "GlQueries.hpp"
#include "GlTypeConversion.hpp"
#include "../ExceptionHelpers.hpp"

namespace Tungsten
{
    bool is_blend_enabled()
    {
        return is_enabled(GL_BLEND);
    }

    void set_blend_enabled(bool enabled)
    {
        if (enabled)
            enable(GL_BLEND);
        else
            disable(GL_BLEND);
    }

    void set_blend_function(BlendFunction src, BlendFunction dst)
    {
        get_ogl_wrapper().blend_func(to_ogl_blend_function(src),
                                     to_ogl_blend_function(dst));
        THROW_IF_GL_ERROR();
    }

    void set_blend_function_separate(BlendFunction src_color,
                                     BlendFunction dst_color,
                                     BlendFunction src_alpha,
                                     BlendFunction dst_alpha)
    {
        get_ogl_wrapper().blend_func_separate(to_ogl_blend_function(src_color),
                                              to_ogl_blend_function(dst_color),
                                              to_ogl_blend_function(src_alpha),
                                              to_ogl_blend_function(dst_alpha));
        THROW_IF_GL_ERROR();
    }

    void set_blend_equation(BlendEquation color, BlendEquation alpha)
    {
        get_ogl_wrapper().blend_equation_separate(to_ogl_blend_equation(color),
                                                  to_ogl_blend_equation(alpha));
        THROW_IF_GL_ERROR();
    }

    bool is_depth_test_enabled()
    {
        return is_enabled(GL_DEPTH_TEST);
    }

    void set_depth_test_enabled(bool enabled)
    {
        set_enabled(GL_DEPTH_TEST, enabled);
    }

    void set_depth_function(CompareFunction func)
    {
        get_ogl_wrapper().depth_func(to_ogl_compare_function(func));
        THROW_IF_GL_ERROR();
    }

    bool is_depth_mask_enabled()
    {
        return get_boolean_value(GL_DEPTH_WRITEMASK);
    }

    void set_depth_mask_enabled(bool enabled)
    {
        get_ogl_wrapper().depth_mask(enabled ? GL_TRUE : GL_FALSE);
        THROW_IF_GL_ERROR();
    }

    bool is_face_culling_enabled()
    {
        return is_enabled(GL_CULL_FACE);
    }

    void set_face_culling_enabled(bool enabled)
    {
        set_enabled(GL_CULL_FACE, enabled);
    }

    void set_face_culling_mode(FaceCullingMode mode)
    {
        get_ogl_wrapper().cull_face(to_ogl_cull_mode(mode));
        THROW_IF_GL_ERROR();
    }

    void set_front_face(FrontFace front_face)
    {
        get_ogl_wrapper().front_face(to_ogl_front_face(front_face));
        THROW_IF_GL_ERROR();
    }

    void set_color_write_mask(ColorWriteMask mask)
    {
        const auto on = [&](ColorWriteMask bit)
        {
            return (mask & bit) != ColorWriteMask::NONE ? GL_TRUE : GL_FALSE;
        };
        get_ogl_wrapper().color_mask(on(ColorWriteMask::RED),
                                     on(ColorWriteMask::GREEN),
                                     on(ColorWriteMask::BLUE),
                                     on(ColorWriteMask::ALPHA));
        THROW_IF_GL_ERROR();
    }

    bool is_multisampling_enabled()
    {
        return is_enabled(GL_MULTISAMPLE);
    }

    void set_multisampling_enabled(bool enabled)
    {
        set_enabled(GL_MULTISAMPLE, enabled);
    }

    bool is_scissor_enabled()
    {
        return is_enabled(GL_SCISSOR_TEST);
    }

    void set_scissor_enabled(bool enabled)
    {
        set_enabled(GL_SCISSOR_TEST, enabled);
    }

    void set_scissor(int x, int y, int width, int height)
    {
        get_ogl_wrapper().scissor(x, y, width, height);
        THROW_IF_GL_ERROR();
    }

    void set_viewport(int x, int y, int width, int height)
    {
        get_ogl_wrapper().viewport(x, y, width, height);
        THROW_IF_GL_ERROR();
    }

    void set_viewport(const Viewport& viewport)
    {
        set_viewport(int(viewport.origin.x()), int(viewport.origin.y()),
                     int(viewport.size.x()), int(viewport.size.y()));
    }

    BlendRestorer::BlendRestorer()
    {
        was_enabled_ = is_blend_enabled();
        if (was_enabled_)
        {
            src_rgb_ = get_int32_value(GL_BLEND_SRC_RGB);
            dst_rgb_ = get_int32_value(GL_BLEND_DST_RGB);
            src_alpha_ = get_int32_value(GL_BLEND_SRC_ALPHA);
            dst_alpha_ = get_int32_value(GL_BLEND_DST_ALPHA);
        }
    }

    BlendRestorer::~BlendRestorer()
    {
        if (was_enabled_)
        {
            get_ogl_wrapper().blend_func_separate(src_rgb_, dst_rgb_, src_alpha_, dst_alpha_);
        }
        set_blend_enabled(was_enabled_);
    }
}
