//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 23.04.2017.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "GlProgram.hpp"

namespace Tungsten
{
    std::string read_shader(const std::string& path);

    ShaderHandle create_shader(GLuint shader_type,
                               const std::string& source_code);

    ShaderHandle read_and_compile_shader(GLuint shader_type,
                                         const std::string& path);
}
