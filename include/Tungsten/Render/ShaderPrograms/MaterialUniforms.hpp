//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "../ColorMaterials.hpp"
#include "Tungsten/Gl/GlUniform.hpp"

namespace Tungsten
{
    class OldShaderProgram;

    struct ColorMaterialUniform
    {
        explicit ColorMaterialUniform(const std::string& prefix,
                                      const OldShaderProgram& shader_program);

        void set(const ColorMaterial& material);

        Uniform<Xyz::Vector3F> ambient;
        Uniform<Xyz::Vector3F> diffuse;
        Uniform<Xyz::Vector3F> specular;
        Uniform<float> shininess;
    };
}
