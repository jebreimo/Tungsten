//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Matrix.hpp>

#include "../Lights.hpp"
#include "Tungsten/Gl/GlUniform.hpp"

namespace Tungsten
{
    class ShaderProgram;

    struct DirectionalLightUniform
    {
        explicit DirectionalLightUniform(const std::string& prefix, const ShaderProgram& shader_program);

        void set(const DirectionalLight& light,
                 const Xyz::Matrix4F& view_matrix);

        Uniform<Xyz::Vector3F> direction;
        Uniform<Xyz::Vector3F> ambient;
        Uniform<Xyz::Vector3F> diffuse;
        Uniform<Xyz::Vector3F> specular;
    };
}
