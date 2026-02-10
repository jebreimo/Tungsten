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

    struct TextureMapMaterial
    {
        Xyz::Vector3F specular = {1.f, 1.f, 1.f};
        float shininess = 32.f;
        int32_t texture_map = 0;
    };

    struct SpecularMapMaterial
    {
        float shininess = 32.f;
        int32_t texture_map = 0;
        int32_t specular_map = 1;
    };

    enum class MaterialType
    {
        COLORED,
        TEXTURED,
        TEXTURED_WITH_SPECULAR_MAP
    };

    struct SmoothShaderFeatures
    {
        MaterialType material_type = MaterialType::COLORED;
        bool use_directional_light = true;
        bool use_point_light = false;
        bool use_spotlight = false;
        unsigned int max_point_lights = 4;
    };

    // struct ShaderFactory
    class SmoothShader : public ShaderProgram
    {
    public:
        static constexpr std::string_view NAME = "builtin::SmoothMesh";

        static std::vector<ShaderFeature> get_features();

        using VertexType = std::tuple<Xyz::Vector3F, Xyz::Vector3F>;

        SmoothShader();

        using ShaderProgram::create_vao;

        [[nodiscard]] VertexArrayObject create_vao(int32_t extra_stride) const override;

        void set_material(const ColoredMaterial& material);

        void set_material(const TextureMapMaterial& material);

        void set_material(const SpecularMapMaterial& material);

        void set_light(const DirectionalLight& light,
                       const Xyz::Matrix4F& view_matrix);

        void set_light(unsigned index,
                       const PointLight& light,
                       const Xyz::Matrix4F& view_matrix);

        void set_light(const Spotlight& light,
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

        struct ColoredMaterialUniforms
        {
            Uniform<Xyz::Vector3F> ambient;
            Uniform<Xyz::Vector3F> diffuse;
            Uniform<Xyz::Vector3F> specular;
            Uniform<float> shininess;
        } colored_material_uniforms;

        struct TextureMapMaterialUniforms
        {
            Uniform<int32_t> texture_map;
            Uniform<Xyz::Vector3F> specular;
            Uniform<float> shininess;
        } textured_material_uniforms;

        struct SpecularMapMaterialUniforms
        {
            Uniform<int32_t> texture_map;
            Uniform<int32_t> specular_map;
            Uniform<float> shininess;
        } specular_map_material_uniforms;

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
