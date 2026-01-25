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
            {"textured_material", "Use textures", "USE_TEXTURES", {}}
        };
    }

    SmoothShader::SmoothShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::BLINN_PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
                        })
    {
        position_attr = get_vertex_attribute(program(), "a_position");
        normal_attr = get_vertex_attribute(program(), "a_normal");
        texture_coord_attr = get_vertex_attribute(program(), "a_texcoord");

        model_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_model_matrix");
        view_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_view_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_proj_matrix");

        material_uniforms = {
            get_uniform<Xyz::Vector3F>(program(), "u_material.ambient"),
            get_uniform<Xyz::Vector3F>(program(), "u_material.diffuse"),
            get_uniform<Xyz::Vector3F>(program(), "u_material.specular"),
            get_uniform<float>(program(), "u_material.shininess")
        };

        dir_light_uniforms = {
            get_uniform<Xyz::Vector3F>(program(), "u_dir_light.direction"),
            get_uniform<Xyz::Vector3F>(program(), "u_dir_light.ambient"),
            get_uniform<Xyz::Vector3F>(program(), "u_dir_light.diffuse"),
            get_uniform<Xyz::Vector3F>(program(), "u_dir_light.specular")
        };

        view_pos = get_uniform<Xyz::Vector3F>(program(), "u_view_pos");
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
        material_uniforms.ambient.set(material.ambient);
        material_uniforms.diffuse.set(material.diffuse);
        material_uniforms.specular.set(material.specular);
        material_uniforms.shininess.set(material.shininess);
    }

    void SmoothShader::set_light(const DirectionalLight& light)
    {
        dir_light_uniforms.direction.set(light.direction);
        dir_light_uniforms.ambient.set(light.ambient);
        dir_light_uniforms.diffuse.set(light.diffuse);
        dir_light_uniforms.specular.set(light.specular);
    }

    void SmoothShader::set_model_matrix(const Xyz::Matrix4F& mv)
    {
        model_matrix.set(mv);
    }

    void SmoothShader::set_camera(const Camera& camera)
    {
        set_view_matrix(camera.view_matrix());
        set_view_pos(camera.view_parameters().position);
        set_projection_matrix(camera.projection_matrix());
    }

    void SmoothShader::set_view_matrix(const Xyz::Matrix4F& mv)
    {
        view_matrix.set(mv);
    }

    void SmoothShader::set_view_pos(const Xyz::Vector3F& pos)
    {
        view_pos.set(pos);
    }

    void SmoothShader::set_projection_matrix(const Xyz::Matrix4F& proj)
    {
        proj_matrix.set(proj);
    }
}
