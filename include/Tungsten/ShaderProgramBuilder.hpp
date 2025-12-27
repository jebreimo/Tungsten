//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-03-28.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include "Gl/GlProgram.hpp"
#include "Gl/GlTypes.hpp"

namespace Tungsten
{
    class ShaderProgramBuilder
    {
    public:
        ShaderProgramBuilder();

        ShaderProgramBuilder& add_shader(ShaderType type, const std::string& src);

        ProgramHandle build();

    private:
        ProgramHandle program_;
    };
}
