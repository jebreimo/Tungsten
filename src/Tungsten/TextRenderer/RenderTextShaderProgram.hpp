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
#include "Tungsten/GlVertexArray.hpp"
#include "Tungsten/ShaderProgram.hpp"
#include "Tungsten/VertexArrayObject.hpp"

namespace Tungsten::Detail
{
    class RenderTextShaderProgram : public ShaderProgram
    {
    public:
        using VertexType = std::tuple<Xyz::Vector2F, Xyz::Vector2F>;

        RenderTextShaderProgram();

        [[nodiscard]] VertexArrayObject create_vao() const override;

        Uniform<Xyz::Matrix4F> mvp_matrix;
        Uniform<int32_t> texture;
        Uniform<Xyz::Vector4F> color;

        uint32_t position;
        uint32_t texture_coord;
    };
}
