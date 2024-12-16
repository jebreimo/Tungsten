//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlShader.hpp"
#include "Uniform.hpp"

namespace Tungsten
{
    struct ProgramDeleter
    {
        void operator()(GLuint id) const;
    };

    using ProgramHandle = GlHandle<ProgramDeleter>;

    ProgramHandle create_program();

    void attach_shader(GLuint program_id, GLuint shader_id);

    GLuint get_vertex_attribute(GLuint program_id, const std::string& name);

    GLint get_uniform_location(GLuint program_id, const char* name);

    template <typename T>
    Uniform<T> get_uniform(GLuint program_id, const char* name)
    {
        return Uniform<T>(get_uniform_location(program_id, name));
    }

    void link_program(GLuint program_id);

    void use_program(GLuint program_id);
}
