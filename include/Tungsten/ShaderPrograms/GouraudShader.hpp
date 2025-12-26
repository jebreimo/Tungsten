//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "SmoothMeshShader.hpp"

namespace Tungsten
{
    class GouraudShader : public SmoothMeshShader
    {
    public:
        static constexpr std::string_view NAME = "builtin::Gouraud";

        GouraudShader();

        void set_light(const Light& light) override;

        Uniform<Xyz::Vector3F> ambient;
    };
}
