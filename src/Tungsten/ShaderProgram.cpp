//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderProgram.hpp"

#include "Tungsten/ShaderProgramBuilder.hpp"

namespace Tungsten
{
    ShaderProgram::ShaderProgram(std::string name,
                                 std::vector<std::pair<ShaderType, std::string>> sources)
        : name_(std::move(name))
    {
        ShaderProgramBuilder builder;
        for (auto& [type, source] : sources)
        {
            builder.add_shader(type, source);
            sources_.insert({type, std::move(source)});
        }
        program_ = builder.build();
    }

    ShaderProgram::~ShaderProgram() = default;

    const std::string& ShaderProgram::name() const
    {
        return name_;
    }

    const std::string& ShaderProgram::source(ShaderType type) const
    {
        return sources_.at(type);
    }

    void ShaderProgram::use() const
    {
        use_program(program_);
    }

    const ProgramHandle& ShaderProgram::program() const
    {
        return program_;
    }
}
