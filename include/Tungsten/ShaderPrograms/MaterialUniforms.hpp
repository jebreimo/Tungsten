//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Materials.hpp"
#include "Tungsten/Gl/GlUniform.hpp"

namespace Tungsten
{
    class ShaderProgram;

    struct ColorMaterialUniform
    {
        explicit ColorMaterialUniform(const ShaderProgram& shader_program);

        void set(const ColorMaterial& material);

        Uniform<Xyz::Vector3F> ambient;
        Uniform<Xyz::Vector3F> diffuse;
        Uniform<Xyz::Vector3F> specular;
        Uniform<float> shininess;
    };
}
