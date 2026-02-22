//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/PhongShader.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"

#include "../ShaderSources.hpp"

namespace Tungsten
{
    PhongShader::PhongShader()
        : ShaderProgram(std::string(NAME),
                        {
                            {ShaderType::VERTEX, ShaderSources::PHONG_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::PHONG_FRAGMENT}
                        })
    {
        position_attr = get_vertex_attribute(handle(), "a_position");
        normal_attr = get_vertex_attribute(handle(), "a_normal");
        set_attribute_definitions({
            {
                position_attr, VertexAttributeType::POSITION_3F
            },
            {
                normal_attr, VertexAttributeType::NORMAL_3F
            }
        });

        mv_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_mv_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_proj_matrix");

        light_pos = get_uniform<Xyz::Vector3F>(handle(), "u_light_pos");
        ambient = get_uniform<Xyz::Vector3F>(handle(), "u_ambient");
        diffuse_albedo = get_uniform<Xyz::Vector3F>(handle(), "u_diffuse_albedo");
        specular_albedo = get_uniform<Xyz::Vector3F>(handle(), "u_specular_albedo");
        specular_power = get_uniform<float>(handle(), "u_specular_power");
    }
}
