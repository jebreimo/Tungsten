//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/GlProgram.hpp"
#include "Tungsten/GlUniform.hpp"

namespace Tungsten::Detail
{
    class RenderTextShaderProgram
    {
    public:
        RenderTextShaderProgram();

        RenderTextShaderProgram(const RenderTextShaderProgram&) = delete;
        RenderTextShaderProgram& operator=(const RenderTextShaderProgram&) = delete;

        ProgramHandle program;

        Uniform<Xyz::Matrix4F> mvp_matrix;
        Uniform<int32_t> texture;
        Uniform<Xyz::Vector4F> color;

        uint32_t position;
        uint32_t texture_coord;
    };
}
