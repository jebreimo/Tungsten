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
#include "../../Camera.hpp"

namespace Tungsten
{
    class SmoothShader : public ShaderProgram
    {
    public:
        static constexpr std::string_view NAME = "builtin::SmoothMesh";

        using VertexType = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;

        SmoothShader();

        void set_model_view_matrix(const Xyz::Matrix4F& mv,
                                   bool update_normal_matrix = true);

        void set_model_view_matrix(const Xyz::Matrix4F& model,
                                   const Xyz::Matrix4F& view,
                                   bool update_normal_matrix = true);

        void set_projection_matrix(const Xyz::Matrix4F& proj);

        void set_normal_matrix(const Xyz::Matrix3F& norm);

        // Runtime feature flags. These replace the former compile-time
        // USE_* defines: the shader is a single program and each light
        // type and material map is switched on or off via these uniforms.
        void set_use_diffuse_map(bool enabled);

        void set_use_specular_map(bool enabled);

        void set_use_directional_light(bool enabled);

        void set_num_point_lights(int count);

        void set_use_spot_light(bool enabled);

        ColorMaterialUniform material;

        DirectionalLightUniform directional_light;
    protected:
        Uniform<Xyz::Matrix4F> model_view_matrix;
        Uniform<Xyz::Matrix3F> normal_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        Uniform<int32_t> use_diffuse_map;
        Uniform<int32_t> use_specular_map;
        Uniform<int32_t> use_directional_light;
        Uniform<int32_t> num_point_lights;
        Uniform<int32_t> use_spot_light;

        uint32_t position_attr;
        uint32_t normal_attr;
        uint32_t texture_coord_attr;
    };
}
