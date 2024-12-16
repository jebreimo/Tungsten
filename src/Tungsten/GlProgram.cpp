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
    void ProgramDeleter::operator()(GLuint id) const
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

    bool get_program_link_status(GLuint program_id)
    {
        GLint result = 0;
        glGetProgramiv(program_id, GL_LINK_STATUS, &result);
        THROW_IF_GL_ERROR();
        return result == GL_TRUE;
    }

    GLint get_program_info_log_length(GLuint program_id)
    {
        GLint size;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &size);
        return size;
    }

    std::string get_program_info_log(GLuint program_id)
    {
        auto size = get_program_info_log_length(program_id);
        std::string result(size, '\0');
        glGetProgramInfoLog(program_id, size, &size, &result[0]);
        return result;
    }

    void link_program(GLuint program_id)
    {
        glLinkProgram(program_id);
        THROW_IF_GL_ERROR();
        if (!get_program_link_status(program_id))
            TUNGSTEN_THROW(+ get_program_info_log(program_id));
    }

    void use_program(GLuint program_id)
    {
        glUseProgram(program_id);
        THROW_IF_GL_ERROR();
    }
}
