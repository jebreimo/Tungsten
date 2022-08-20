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
    void GlShaderDeleter::operator()(GLuint id) const
    {
        glDeleteShader(id);
        THROW_IF_GL_ERROR();
    }

    void compile_shader(GLuint shader_id)
    {
        glCompileShader(shader_id);
        THROW_IF_GL_ERROR();
        if (!get_shader_compile_status(shader_id))
            TUNGSTEN_THROW(get_shader_info_log(shader_id));
    }

    ShaderHandle create_shader(GLuint shader_type)
    {
        auto id = glCreateShader(shader_type);
        THROW_IF_GL_ERROR();
        return ShaderHandle(id);
    }

    bool get_shader_compile_status(GLuint shader_id)
    {
        GLint result = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    GLint get_shader_info_log_length(GLuint shader_id)
    {
        GLint size;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_shader_info_log(GLuint shader_id)
    {
        auto size = get_shader_info_log_length(shader_id);
        std::string result(size, '\0');
        glGetShaderInfoLog(shader_id, size, &size, &result[0]);
        return result;
    }

    void set_shader_source(GLuint shader_id, const std::string& source)
    {
        auto length = (GLint)source.size();
        auto str = source.c_str();
        glShaderSource(shader_id, 1, &str, &length);
        THROW_IF_GL_ERROR();
    }
}
