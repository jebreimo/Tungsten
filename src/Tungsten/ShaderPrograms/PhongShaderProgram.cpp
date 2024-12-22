//****************************************************************************
// Copyright © 2022 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2022-06-11.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "PhongShaderProgram.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "Phong-frag.glsl.hpp"
#include "Phong-vert.glsl.hpp"

namespace Tungsten
{
    void PhongShaderProgram::setup()
    {
        program = ShaderProgramBuilder()
            .add_shader(ShaderType::VERTEX, Phong_vert)
            .add_shader(ShaderType::FRAGMENT, Phong_frag)
            .build();

        position_attr = get_vertex_attribute(program, "a_position");
        normal_attr = get_vertex_attribute(program, "a_normal");

        mv_matrix = get_uniform<Xyz::Matrix4F>(program, "u_mv_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(program, "u_proj_matrix");

        light_pos = get_uniform<Xyz::Vector3F>(program, "u_light_pos");
        diffuse_albedo = get_uniform<Xyz::Vector3F>(program, "u_diffuse_albedo");
        specular_albedo = get_uniform<Xyz::Vector3F>(program, "u_specular_albedo");
        specular_power = get_uniform<float>(program, "u_specular_power");
    }
}