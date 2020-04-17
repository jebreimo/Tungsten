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

    void compileShader(GLuint shaderId)
    {
        glCompileShader(shaderId);
        THROW_IF_GL_ERROR();
        if (!getShaderCompileStatus(shaderId))
            TUNGSTEN_THROW(getShaderInfoLog(shaderId));
    }

    ShaderHandle createShader(GLuint shaderType)
    {
        auto id = glCreateShader(shaderType);
        THROW_IF_GL_ERROR();
        return ShaderHandle(id);
    }

    bool getShaderCompileStatus(GLuint shaderId)
    {
        GLint result = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    GLint getShaderInfoLogLength(GLuint shaderId)
    {
        GLint size;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string getShaderInfoLog(GLuint shaderId)
    {
        auto size = getShaderInfoLogLength(shaderId);
        std::string result(size, '\0');
        glGetShaderInfoLog(shaderId, size, &size, &result[0]);
        return result;
    }

    void setShaderSource(GLuint shaderId, const std::string& source)
    {
        auto length = (GLint)source.size();
        auto str = source.c_str();
        glShaderSource(shaderId, 1, &str, &length);
        THROW_IF_GL_ERROR();
    }
}
