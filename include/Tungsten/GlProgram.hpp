//****************************************************************************
// Copyright © 2017 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2017-05-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlHandle.hpp"
#include "Uniform.hpp"

namespace Tungsten
{
    struct GlShaderDeleter
    {
        void operator()(GLuint id) const;
    };

    using ShaderHandle = GlHandle<GlShaderDeleter>;

    struct GlProgramDeleter
    {
        void operator()(GLuint id) const;
    };

    using ProgramHandle = GlHandle<GlProgramDeleter>;

    void compileShader(GLuint shaderId);

    ShaderHandle createShader(GLuint shaderType);

    void setShaderSource(GLuint shaderId, const std::string& source);

    bool getShaderCompileStatus(GLuint shaderId);

    std::string getShaderInfoLog(GLuint shaderId);

    GLint getShaderInfoLogLength(GLuint shaderId);

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
