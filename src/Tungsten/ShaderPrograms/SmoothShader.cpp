//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/SmoothShader.hpp"

#include "Tungsten/VertexArrayObjectBuilder.hpp"
#include "../ShaderSources.hpp"

namespace Tungsten
{
    std::vector<ShaderFeature> SmoothShader::get_features()
    {
        return {
            {"textured_material", "Use textures", "USE_TEXTURES", {}},
            {"directional_light", "Use directional light", "USE_DIRECTIONAL_LIGHT", {}},
            {"point_light", "Use point light", "USE_POINT_LIGHT", {}},
            {"spotlight", "Use spotlight", "USE_SPOTLIGHT", {}}
        };
    }

    SmoothShader::SmoothShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::BLINN_PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
                        },
                        ShaderPreprocessor().add_define("USE_DIRECTIONAL_LIGHT"))
    {
        position_attr = get_vertex_attribute(handle(), "a_position");
        normal_attr = get_vertex_attribute(handle(), "a_normal");
        texture_coord_attr = get_vertex_attribute(handle(), "a_tex_coord");

        model_view_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_model_view_matrix");
        normal_matrix = get_uniform<Xyz::Matrix3F>(handle(), "u_normal_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_proj_matrix");

        colored_material_uniforms = {
            get_uniform<Xyz::Vector3F>(handle(), "u_material.ambient"),
            get_uniform<Xyz::Vector3F>(handle(), "u_material.diffuse"),
            get_uniform<Xyz::Vector3F>(handle(), "u_material.specular"),
            get_uniform<float>(handle(), "u_material.shininess")
        };

        dir_light_uniforms = {
            get_uniform<Xyz::Vector3F>(handle(), "u_dir_light.direction"),
            get_uniform<Xyz::Vector3F>(handle(), "u_dir_light.ambient"),
            get_uniform<Xyz::Vector3F>(handle(), "u_dir_light.diffuse"),
            get_uniform<Xyz::Vector3F>(handle(), "u_dir_light.specular")
        };

        view_pos = get_uniform<Xyz::Vector3F>(handle(), "u_view_pos");
    }

    VertexArrayObject SmoothShader::create_vao(int32_t extra_stride) const
    {
        return VertexArrayObjectBuilder()
            .add_float(position_attr, 3)
            .add_float(normal_attr, 3)
            .add_stride(extra_stride)
            .build();
    }

    void SmoothShader::set_material(const ColoredMaterial& material)
    {
        colored_material_uniforms.ambient.set(material.ambient);
        colored_material_uniforms.diffuse.set(material.diffuse);
        colored_material_uniforms.specular.set(material.specular);
        colored_material_uniforms.shininess.set(material.shininess);
    }

    void SmoothShader::set_light(const DirectionalLight& light,
                                 const Xyz::Matrix4F& view_matrix)
    {
        dir_light_uniforms.direction.set(Xyz::make_submatrix<3, 3>(view_matrix) * light.direction);
        dir_light_uniforms.ambient.set(light.light.ambient);
        dir_light_uniforms.diffuse.set(light.light.diffuse);
        dir_light_uniforms.specular.set(light.light.specular);
    }

    void SmoothShader::set_model_view_matrix(const Xyz::Matrix4F& mv,
                                             bool update_normal_matrix)
    {
        model_view_matrix.set(mv);
        if (update_normal_matrix)
        {
            normal_matrix.set(Xyz::make_submatrix<3, 3>(invert(mv)), false);
        }
    }

    void SmoothShader::set_model_view_matrix(const Xyz::Matrix4F& model,
                                             const Xyz::Matrix4F& view,
                                             bool update_normal_matrix)
    {
        set_model_view_matrix(view * model, update_normal_matrix);
    }

    void SmoothShader::set_projection_matrix(const Xyz::Matrix4F& proj)
    {
        proj_matrix.set(proj);
    }

    void SmoothShader::set_normal_matrix(const Xyz::Matrix3F& norm)
    {
        normal_matrix.set(norm);
    }
}
