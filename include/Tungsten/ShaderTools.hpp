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
    std::string readShader(const std::string& path);

    ShaderHandle createShader(GLuint shaderType,
                              const std::string& sourceCode);

    ShaderHandle readAndCompileShader(GLuint shaderType,
                                      const std::string& path);
}
