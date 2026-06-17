//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "Tungsten/ShaderTools.hpp"
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    ShaderProgramBuilder::ShaderProgramBuilder()
        : program_(create_program())
    {}

    ShaderProgramBuilder&
    ShaderProgramBuilder::add_shader(ShaderType type, const std::string& src)
    {
        auto shader = create_shader(type, src);
        attach_shader(program_.id(), shader.id());
        return *this;
    }

    ProgramHandle ShaderProgramBuilder::build()
    {
        link_program(program_.id());
        return std::move(program_);
    }
}
