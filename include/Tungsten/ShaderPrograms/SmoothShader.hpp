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

namespace Tungsten
{
    struct Material
    {
        Xyz::Vector3F ambient = {0.5, 0.2, 0.7};
        Xyz::Vector3F diffuse = {0.5, 0.2, 0.7};
        Xyz::Vector3F specular = {0.7, 0.7, 0.7};
        float shininess = 128.0f;
    };

    struct DirectionalLight
    {
        Xyz::Vector3F direction = {1.0f, 1.0f, -10.0f};
        Xyz::Vector3F ambient = {0.1f, 0.1f, 0.1f};
        Xyz::Vector3F diffuse = {0.5f, 0.5f, 0.5f};
        Xyz::Vector3F specular = {0.7, 0.7, 0.7};
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

        virtual void set_material(const Material& material);

        virtual void set_light(const DirectionalLight& light);

        void set_model_matrix(const Xyz::Matrix4F& mv);

        void set_camera(const Camera& camera);

        void set_view_matrix(const Xyz::Matrix4F& mv);

        void set_view_pos(const Xyz::Vector3F& pos);

        void set_projection_matrix(const Xyz::Matrix4F& proj);

    protected:
        Uniform<Xyz::Matrix4F> model_matrix;
        Uniform<Xyz::Matrix4F> view_matrix;
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
