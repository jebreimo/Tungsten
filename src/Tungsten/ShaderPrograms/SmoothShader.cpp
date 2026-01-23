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
            {"Use textures", "USE_TEXTURES", {}}
        };
    }

    SmoothShader::SmoothShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
                        })
    {
        position_attr = get_vertex_attribute(program(), "a_position");
        normal_attr = get_vertex_attribute(program(), "a_normal");
        texture_coord_attr = get_vertex_attribute(program(), "a_texcoord");

        mv_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_mv_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_proj_matrix");

        light_pos = get_uniform<Xyz::Vector3F>(program(), "u_light_pos");
        ambient = get_uniform<Xyz::Vector3F>(program(), "u_ambient");
        diffuse_albedo = get_uniform<Xyz::Vector3F>(program(), "u_diffuse_albedo");
        specular_albedo = get_uniform<Xyz::Vector3F>(program(), "u_specular_albedo");
        specular_power = get_uniform<float>(program(), "u_specular_power");
        texture = get_uniform<int32_t>(program(), "u_texture");
    }

    VertexArrayObject SmoothShader::create_vao(int32_t extra_stride) const
    {
        return VertexArrayObjectBuilder()
            .add_float(position_attr, 3)
            .add_float(normal_attr, 3)
            .add_stride(extra_stride)
            .build();
    }

    void SmoothShader::set_material(const Material& material)
    {
        diffuse_albedo.set(material.diffuse_albedo);
        specular_albedo.set(material.specular_albedo);
        specular_power.set(material.specular_exponent);
    }

    void SmoothShader::set_light(const Light& light)
    {
        light_pos.set(light.position);
    }

    void SmoothShader::set_model_view_matrix(const Xyz::Matrix4F& mv)
    {
        mv_matrix.set(mv);
    }

    void SmoothShader::set_projection_matrix(const Xyz::Matrix4F& proj)
    {
        proj_matrix.set(proj);
    }
}
