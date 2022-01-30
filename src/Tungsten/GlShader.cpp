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

    void compile_shader(GLuint shaderId)
    {
        glCompileShader(shaderId);
        THROW_IF_GL_ERROR();
        if (!get_shader_compile_status(shaderId))
            TUNGSTEN_THROW(get_shader_info_log(shaderId));
    }

    ShaderHandle create_shader(GLuint shaderType)
    {
        auto id = glCreateShader(shaderType);
        THROW_IF_GL_ERROR();
        return ShaderHandle(id);
    }

    bool get_shader_compile_status(GLuint shaderId)
    {
        GLint result = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    GLint get_shader_info_log_length(GLuint shaderId)
    {
        GLint size;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_shader_info_log(GLuint shaderId)
    {
        auto size = get_shader_info_log_length(shaderId);
        std::string result(size, '\0');
        glGetShaderInfoLog(shaderId, size, &size, &result[0]);
        return result;
    }

    void set_shader_source(GLuint shaderId, const std::string& source)
    {
        auto length = (GLint)source.size();
        auto str = source.c_str();
        glShaderSource(shaderId, 1, &str, &length);
        THROW_IF_GL_ERROR();
    }
}
