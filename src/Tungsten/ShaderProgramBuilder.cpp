//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-28.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "Tungsten/ShaderTools.hpp"

namespace Tungsten
{
    namespace
    {
        GLenum to_gl_type(ShaderType type)
        {
            switch (type)
            {
            case ShaderType::VERTEX:
                return GL_VERTEX_SHADER;
            case ShaderType::FRAGMENT:
                return GL_FRAGMENT_SHADER;
            case ShaderType::COMPUTE:
                return GL_COMPUTE_SHADER;
            }
        }
    }

    ShaderProgramBuilder::ShaderProgramBuilder()
        : program_(Tungsten::create_program())
    {}

    ShaderProgramBuilder&
    ShaderProgramBuilder::add_shader(ShaderType type, const std::string& src)
    {
        auto shader = Tungsten::create_shader(to_gl_type(type), src);
        Tungsten::attach_shader(program_, shader);
        return *this;
    }

    Tungsten::ProgramHandle ShaderProgramBuilder::build()
    {
        Tungsten::link_program(program_);
        return std::move(program_);
    }
}
