//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ShaderProgram.hpp"

namespace Tungsten
{
    class GouraudShader : public ShaderProgram
    {
    public:
        static constexpr std::string_view NAME = "builtin::Gouraud";

        GouraudShader();

        [[nodiscard]] VertexArrayObject create_vao(int32_t extra_stride) const override;

        Uniform<Xyz::Matrix4F> mv_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        Uniform<Xyz::Vector3F> light_pos;

        Uniform<Xyz::Vector3F> ambient;
        Uniform<Xyz::Vector3F> diffuse_albedo;
        Uniform<Xyz::Vector3F> specular_albedo;
        Uniform<float> specular_power;

        uint32_t position_attr;
        uint32_t normal_attr;
    };
}
