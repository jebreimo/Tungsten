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

    bool is_depth_test_enabled()
    {
        return is_enabled(GL_DEPTH_TEST);
    }

    void set_depth_test_enabled(bool enabled)
    {
        set_enabled(GL_DEPTH_TEST, enabled);
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

    bool is_multisampling_enabled()
    {
        return is_enabled(GL_MULTISAMPLE);
    }

    void set_multisampling_enabled(bool enabled)
    {
        set_enabled(GL_MULTISAMPLE, enabled);
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

    bool get_boolean_value(unsigned parameter_name)
    {
        GLboolean value;
        get_ogl_wrapper().get_boolean(parameter_name, &value);
        THROW_IF_GL_ERROR();
        return value;
    }

    float get_float_value(unsigned parameter_name)
    {
        float value;
        get_ogl_wrapper().get_float(parameter_name, &value);
        THROW_IF_GL_ERROR();
        return value;
    }

    int32_t get_int32_value(unsigned parameter_name)
    {
        int32_t value;
        get_ogl_wrapper().get_integer(parameter_name, &value);
        THROW_IF_GL_ERROR();
        return value;
    }

    int64_t get_int64_value(unsigned parameter_name)
    {
        int64_t value;
        get_ogl_wrapper().get_integer64(parameter_name, &value);
        THROW_IF_GL_ERROR();
        return value;
    }

    std::string get_string_value(unsigned parameter_name)
    {
        const auto str = reinterpret_cast<const char*>(get_ogl_wrapper().
            get_string(parameter_name));
        THROW_IF_GL_ERROR();
        return str ? str : "";
    }

    bool is_enabled(unsigned capability)
    {
        const auto result = get_ogl_wrapper().is_enabled(capability) != 0;
        THROW_IF_GL_ERROR();
        return result;
    }

    void set_enabled(unsigned capability, bool enabled)
    {
        if (enabled)
            enable(capability);
        else
            disable(capability);
    }

    void enable(unsigned capability)
    {
        get_ogl_wrapper().enable(capability);
        THROW_IF_GL_ERROR();
    }

    void disable(unsigned capability)
    {
        get_ogl_wrapper().disable(capability);
        THROW_IF_GL_ERROR();
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
