//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GlQueries.hpp"

#include "Tungsten/Gl/IOglWrapper.hpp"
#include "../ExceptionHelpers.hpp"

namespace Tungsten
{
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
        // Zero-initialized because the dummy and Emscripten backends may
        // leave the output untouched; returning an indeterminate value from a
        // query that "succeeded" is worse than returning zero.
        int32_t value = 0;
        get_ogl_wrapper().get_integer(parameter_name, &value);
        THROW_IF_GL_ERROR();
        return value;
    }

    int64_t get_int64_value(unsigned parameter_name)
    {
        int64_t value = 0;
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
}
