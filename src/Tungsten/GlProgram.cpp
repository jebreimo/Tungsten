//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/GlProgram.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    void GlProgramDeleter::operator()(GLuint id) const
    {
        glDeleteProgram(id);
        THROW_IF_GL_ERROR();
    }

    ProgramHandle createProgram()
    {
        auto id = glCreateProgram();
        THROW_IF_GL_ERROR();
        return ProgramHandle(id);
    }

    void attachShader(GLuint programId, GLuint shaderId)
    {
        glAttachShader(programId, shaderId);
        THROW_IF_GL_ERROR();
    }

    GLuint getVertexAttribute(GLuint programId, const std::string& name)
    {
        auto attrLoc = glGetAttribLocation(programId, name.c_str());
        THROW_IF_GL_ERROR();
        return GLuint(attrLoc);
    }

    GLint getUniformLocation(GLuint programId, const char* name)
    {
        auto result = glGetUniformLocation(programId, name);
        THROW_IF_GL_ERROR();
        return result;
    }

    bool getProgramLinkStatus(GLuint programId)
    {
        GLint result = 0;
        glGetProgramiv(programId, GL_LINK_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    GLint getProgramInfoLogLength(GLuint programId)
    {
        GLint size;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string getProgramInfoLog(GLuint programId)
    {
        auto size = getProgramInfoLogLength(programId);
        std::string result(size, '\0');
        glGetProgramInfoLog(programId, size, &size, &result[0]);
        return result;
    }

    void linkProgram(GLuint programId)
    {
        glLinkProgram(programId);
        THROW_IF_GL_ERROR();
        if (!getProgramLinkStatus(programId))
            TUNGSTEN_THROW(getProgramInfoLog(programId));
    }

    void useProgram(GLuint programId)
    {
        glUseProgram(programId);
        THROW_IF_GL_ERROR();
    }
}
