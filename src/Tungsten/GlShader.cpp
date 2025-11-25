//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlShader.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void ShaderDeleter::operator()(uint32_t id) const
    {
        glDeleteShader(id);
        THROW_IF_GL_ERROR();
    }

    void compile_shader(uint32_t shader_id)
    {
        glCompileShader(shader_id);
        THROW_IF_GL_ERROR();
        if (!get_shader_compile_status(shader_id))
            TUNGSTEN_THROW(+ get_shader_info_log(shader_id));
    }

    ShaderHandle create_shader(uint32_t shader_type)
    {
        auto id = glCreateShader(shader_type);
        THROW_IF_GL_ERROR();
        return ShaderHandle(id);
    }

    bool get_shader_compile_status(uint32_t shader_id)
    {
        int32_t result = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    int32_t get_shader_info_log_length(uint32_t shader_id)
    {
        int32_t size;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_shader_info_log(uint32_t shader_id)
    {
        auto size = get_shader_info_log_length(shader_id);
        std::string result(size, '\0');
        glGetShaderInfoLog(shader_id, size, &size, &result[0]);
        return result;
    }

    void set_shader_source(uint32_t shader_id, const std::string& source)
    {
        auto length = (int32_t)source.size();
        auto str = source.c_str();
        glShaderSource(shader_id, 1, &str, &length);
        THROW_IF_GL_ERROR();
    }
}
