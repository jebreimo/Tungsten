//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/GlProgram.hpp"
#include "Tungsten/Uniform.hpp"

namespace Tungsten
{
    class RenderTextShaderProgram
    {
    public:
        RenderTextShaderProgram();

        RenderTextShaderProgram(const RenderTextShaderProgram&) = delete;
        RenderTextShaderProgram& operator=(const RenderTextShaderProgram&) = delete;

        Tungsten::ProgramHandle program;

        Tungsten::Uniform<Xyz::Matrix4F> mvp_matrix;
        Tungsten::Uniform<GLint> texture;
        Tungsten::Uniform<Xyz::Vector4F> color;

        GLuint position;
        GLuint texture_coord;
    };
}
