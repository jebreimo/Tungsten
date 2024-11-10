//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/GlProgram.hpp"

namespace Tungsten
{
    class GouraudShaderProgram
    {
    public:
        void setup();

        ProgramHandle program;

        Uniform<Xyz::Matrix4F> mv_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        Uniform<Xyz::Vector3F> light_pos;
        Uniform<Xyz::Vector3F> diffuse_albedo;
        Uniform<Xyz::Vector3F> specular_albedo;
        Uniform<float> specular_power;
        Uniform<Xyz::Vector3F> ambient;

        GLuint position_attr;
        GLuint normal_attr;
    };
}
