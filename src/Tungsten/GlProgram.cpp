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

    ProgramHandle create_program()
    {
        auto id = glCreateProgram();
        THROW_IF_GL_ERROR();
        return ProgramHandle(id);
    }

    void attach_shader(GLuint program_id, GLuint shader_id)
    {
        glAttachShader(program_id, shader_id);
        THROW_IF_GL_ERROR();
    }

    GLuint get_vertex_attribute(GLuint program_id, const std::string& name)
    {
        auto attrLoc = glGetAttribLocation(program_id, name.c_str());
        THROW_IF_GL_ERROR();
        return GLuint(attrLoc);
    }

    GLint get_uniform_location(GLuint program_id, const char* name)
    {
        auto result = glGetUniformLocation(program_id, name);
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

    void link_program(GLuint program_id)
    {
        glLinkProgram(program_id);
        THROW_IF_GL_ERROR();
        if (!getProgramLinkStatus(program_id))
            TUNGSTEN_THROW(getProgramInfoLog(program_id));
    }

    void use_program(GLuint program_id)
    {
        glUseProgram(program_id);
        THROW_IF_GL_ERROR();
    }
}
