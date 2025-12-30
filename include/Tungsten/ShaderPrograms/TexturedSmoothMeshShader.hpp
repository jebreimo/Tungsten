//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "SmoothMeshShader.hpp"

namespace Tungsten
{
    class TexturedSmoothMeshShader : public SmoothMeshShader
    {
    public:
        using VertexType = std::tuple<Xyz::Vector3F, Xyz::Vector3F, Xyz::Vector2F>;

        using ShaderProgram::create_vao;

        [[nodiscard]] VertexArrayObject create_vao(int32_t extra_stride) const override;

        void set_texture(int32_t texture_handle);
    protected:
        using SmoothMeshShader::SmoothMeshShader;

        Uniform<int32_t> texture;
        uint32_t uv_attr;
    };
}
