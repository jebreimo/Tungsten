//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ShaderProgram.hpp"

namespace Tungsten
{
    struct Material
    {
        Xyz::Vector3F diffuse_albedo = {0.5, 0.2, 0.7};
        Xyz::Vector3F specular_albedo = {0.7, 0.7, 0.7};
        float specular_exponent = 128.0f;
    };

    struct Light
    {
        Xyz::Vector3F position = {100.0f, 100.0f, 100.0f};
        Xyz::Vector3F ambient = {0.1f, 0.1f, 0.1f};
    };

    struct ShaderFeature
    {
        std::string name;
        std::string definition_name;
        std::string definition_value;
    };

    class SmoothShader : public ShaderProgram
    {
    public:
        static constexpr std::string_view NAME = "builtin::SmoothMesh";

        static std::vector<ShaderFeature> get_features();

        using VertexType = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;

        SmoothShader();

        using ShaderProgram::create_vao;

        [[nodiscard]] VertexArrayObject create_vao(int32_t extra_stride) const override;

        virtual void set_material(const Material& material);

        virtual void set_light(const Light& light);

        void set_model_view_matrix(const Xyz::Matrix4F& mv);

        void set_projection_matrix(const Xyz::Matrix4F& proj);

    protected:
        Uniform<Xyz::Matrix4F> mv_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        Uniform<Xyz::Vector3F> light_pos;

        Uniform<Xyz::Vector3F> ambient;
        Uniform<Xyz::Vector3F> diffuse_albedo;
        Uniform<Xyz::Vector3F> specular_albedo;
        Uniform<float> specular_power;
        Uniform<int32_t> texture;

        uint32_t position_attr;
        uint32_t normal_attr;
        uint32_t texture_coord_attr;

    };
}
