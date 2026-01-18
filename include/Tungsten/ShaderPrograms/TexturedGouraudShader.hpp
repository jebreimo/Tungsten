//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "TexturedSmoothMeshShader.hpp"

namespace Tungsten
{
    class TexturedGouraudShader : public TexturedSmoothMeshShader
    {
    public:
        static constexpr std::string_view NAME = "builtin::TexturedGouraud";

        TexturedGouraudShader();

        void set_light(const Light& light) override;

        Uniform<Xyz::Vector3F> ambient;
    };
}
