//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ShaderProgram.hpp"
#include "Tungsten/Camera.hpp"
#include "Tungsten/ColoredMaterial.hpp"

namespace Tungsten
{
    struct Light
    {
        Xyz::Vector3F ambient = {1.f, 1.f, 1.f};
        Xyz::Vector3F diffuse = {1.f, 1.f, 1.f};
        Xyz::Vector3F specular = {1.f, 1.f, 1.f};
    };

    struct DirectionalLight
    {
        Xyz::Vector3F direction = {1.0f, -1.0f, -10.0f};
        Light light;
    };

    struct LightAttenuation
    {
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
    };

    struct PointLight
    {
        Xyz::Vector3F position = {0.0f, 0.0f, 0.0f};
        Light light;
        LightAttenuation attenuation;
    };

    struct Spotlight
    {
        Xyz::Vector3F position = {0.0f, 0.0f, 0.0f};
        Xyz::Vector3F direction = {0.0f, 0.0f, -1.0f};
        Light light;
        LightAttenuation attenuation;
        float cut_off = 12.5f; // degrees
        float outer_cut_off = 15.0f; // degrees
    };

    struct ShaderFeature
    {
        std::string name;
        std::string description;
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

        virtual void set_material(const ColoredMaterial& material);

        virtual void set_light(const DirectionalLight& light,
                               const Xyz::Matrix4F& view_matrix);

        void set_model_view_matrix(const Xyz::Matrix4F& mv,
                                   bool update_normal_matrix = true);

        void set_model_view_matrix(const Xyz::Matrix4F& model,
                                   const Xyz::Matrix4F& view,
                                   bool update_normal_matrix = true);

        void set_projection_matrix(const Xyz::Matrix4F& proj);

        void set_normal_matrix(const Xyz::Matrix3F& norm);

    protected:
        Uniform<Xyz::Matrix4F> model_view_matrix;
        Uniform<Xyz::Matrix3F> normal_matrix;
        Uniform<Xyz::Matrix4F> proj_matrix;

        struct MaterialUniforms
        {
            Uniform<Xyz::Vector3F> ambient;
            Uniform<Xyz::Vector3F> diffuse;
            Uniform<Xyz::Vector3F> specular;
            Uniform<float> shininess;
        } material_uniforms;

        struct DirectionalLightUniforms
        {
            Uniform<Xyz::Vector3F> direction;
            Uniform<Xyz::Vector3F> ambient;
            Uniform<Xyz::Vector3F> diffuse;
            Uniform<Xyz::Vector3F> specular;
        } dir_light_uniforms;

        Uniform<Xyz::Vector3F> view_pos;

        uint32_t position_attr;
        uint32_t normal_attr;
        uint32_t texture_coord_attr;
    };
}
