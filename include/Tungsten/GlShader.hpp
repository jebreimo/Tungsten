//****************************************************************************
// Copyright © 2019 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2019-07-19.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "GlHandle.hpp"

namespace Tungsten
{
    struct GlShaderDeleter
    {
        void operator()(GLuint id) const;
    };

    using ShaderHandle = GlHandle<GlShaderDeleter>;

    void compileShader(GLuint shaderId);

    ShaderHandle createShader(GLuint shaderType);

    void setShaderSource(GLuint shaderId, const std::string& source);

    bool getShaderCompileStatus(GLuint shaderId);

    std::string getShaderInfoLog(GLuint shaderId);

    GLint getShaderInfoLogLength(GLuint shaderId);
}
