//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Gl/GlShader.hpp"

#include "GlTypeConversion.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void ShaderDeleter::operator()(uint32_t id) const
    {
        get_ogl_wrapper().delete_shader(id);
        THROW_IF_GL_ERROR();
    }

    void compile_shader(uint32_t shader_id)
    {
        get_ogl_wrapper().compile_shader(shader_id);
        THROW_IF_GL_ERROR();
        if (!get_shader_compile_status(shader_id))
            TUNGSTEN_THROW(+ get_shader_info_log(shader_id));
    }

    ShaderHandle create_shader(ShaderType shader_type)
    {
        const auto id = get_ogl_wrapper().create_shader(to_ogl_shader_type(shader_type));
        THROW_IF_GL_ERROR();
        return ShaderHandle(id);
    }

    bool get_shader_compile_status(uint32_t shader_id)
    {
        int32_t result = 0;
        get_ogl_wrapper().get_shader(shader_id, GL_COMPILE_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    int32_t get_shader_info_log_length(uint32_t shader_id)
    {
        int32_t size;
        get_ogl_wrapper().get_shader(shader_id, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_shader_info_log(uint32_t shader_id)
    {
        auto size = get_shader_info_log_length(shader_id);
        std::string result(size, '\0');
        get_ogl_wrapper().get_shader_info_log(shader_id, size, &size, result.data());
        return result;
    }

    void set_shader_source(uint32_t shader_id, const std::string& source)
    {
        const auto length = static_cast<int32_t>(source.size());
        const auto str = source.c_str();
        get_ogl_wrapper().shader_source(shader_id, 1, &str, &length);
        THROW_IF_GL_ERROR();
    }
}
