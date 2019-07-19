//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlShader.hpp"
#include "Uniform.hpp"

namespace Tungsten
{
    struct GlProgramDeleter
    {
        void operator()(GLuint id) const;
    };

    using ProgramHandle = GlHandle<GlProgramDeleter>;

    ProgramHandle createProgram();

    void attachShader(GLuint programId, GLuint shaderId);

    GLuint getVertexAttribute(GLuint programId, const std::string& name);

    GLint getUniformLocation(GLuint programId, const char* name);

    template <typename T>
    Uniform<T> getUniform(GLuint programId, const char* name)
    {
        return Uniform<T>(getUniformLocation(programId, name));
    }

    void linkProgram(GLuint programId);

    void useProgram(GLuint programId);
}
