//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "LightUniforms.hpp"
#include "MaterialUniforms.hpp"
#include "ShaderProgram.hpp"
#include "Tungsten/Camera.hpp"

namespace Tungsten
{
    struct ShaderFeature
    {
        std::string name;
        std::string description;
        std::string definition_name;
        std::string definition_value;
    };

    enum class MaterialType
    {
        COLORED,
        DIFFUSE_MAP,
        SPECULAR_MAP,
        DIFFUSE_SPECULAR_MAP
    };

    struct SmoothShaderFeatures
    {
        MaterialType material_type = MaterialType::COLORED;
        bool use_directional_light = true;
        bool use_point_light = false;
        bool use_spotlight = false;
        unsigned int max_point_lights = 4;
    };

    class SmoothShader : public ShaderProgram
    {
    public:
        static constexpr std::string_view NAME = "builtin::SmoothMesh";

        static std::vector<ShaderFeature> get_features();

        using VertexType = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;

        SmoothShader();

        void set_model_view_matrix(const Xyz::Matrix4F& mv,
                                   bool update_normal_matrix = true);

        void set_model_view_matrix(const Xyz::Matrix4F& model,
                                   const Xyz::Matrix4F& view,
                                   bool update_normal_matrix = true);

        void set_projection_matrix(const Xyz::Matrix4F& proj);

        void set_normal_matrix(const Xyz::Matrix3F& norm);

        ColorMaterialUniform material;

        DirectionalLightUniform directional_light;
    protected:
        Uniform<Xyz::Matrix4F> model_view_matrix;
        Uniform<Xyz::Matrix3F> normal_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        uint32_t position_attr;
        uint32_t normal_attr;
        uint32_t texture_coord_attr;
    };
}
