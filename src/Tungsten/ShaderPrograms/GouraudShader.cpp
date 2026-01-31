//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/GouraudShader.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"

#include "../ShaderSources.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"

namespace Tungsten
{
    GouraudShader::GouraudShader()
        : ShaderProgram(std::string(NAME),
                           {
                               {ShaderType::VERTEX, ShaderSources::GOURAUD_VERTEX},
                               {ShaderType::FRAGMENT, ShaderSources::GOURAUD_FRAGMENT}
                           })
    {
        position_attr = get_vertex_attribute(handle(), "a_position");
        normal_attr = get_vertex_attribute(handle(), "a_normal");

        mv_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_mv_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(handle(), "u_proj_matrix");

        light_pos = get_uniform<Xyz::Vector3F>(handle(), "u_light_pos");
        diffuse_albedo = get_uniform<Xyz::Vector3F>(handle(), "u_diffuse_albedo");
        specular_albedo = get_uniform<Xyz::Vector3F>(handle(), "u_specular_albedo");
        specular_power = get_uniform<float>(handle(), "u_specular_power");
        ambient = get_uniform<Xyz::Vector3F>(handle(), "u_ambient");
    }

    VertexArrayObject GouraudShader::create_vao(int32_t extra_stride) const
    {
        return VertexArrayObjectBuilder()
            .add_float(position_attr, 3)
            .add_float(normal_attr, 3)
            .add_stride(extra_stride)
            .build();
    }
}
