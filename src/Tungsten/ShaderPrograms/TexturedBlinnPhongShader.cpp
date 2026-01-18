//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/TexturedBlinnPhongShader.hpp"
#include "Tungsten/ShaderSources.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"

namespace Tungsten
{
    TexturedBlinnPhongShader::TexturedBlinnPhongShader()
        : TexturedSmoothMeshShader(
              std::string(NAME),
              {
                  {ShaderType::VERTEX, ShaderSources::PHONG_VERTEX},
                  {ShaderType::FRAGMENT, ShaderSources::BLINN_PHONG_FRAGMENT}
              },
              ShaderPreprocessor().add_define("USE_TEXTURES"))
    {
        position_attr = get_vertex_attribute(program(), "a_position");
        normal_attr = get_vertex_attribute(program(), "a_normal");
        uv_attr = get_vertex_attribute(program(), "a_texcoord");

        mv_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_mv_matrix");
        proj_matrix = get_uniform<Xyz::Matrix4F>(program(), "u_proj_matrix");

        light_pos = get_uniform<Xyz::Vector3F>(program(), "u_light_pos");
        diffuse_albedo = get_uniform<Xyz::Vector3F>(program(), "u_diffuse_albedo");
        specular_albedo = get_uniform<Xyz::Vector3F>(program(), "u_specular_albedo");
        specular_power = get_uniform<float>(program(), "u_specular_power");
        texture = get_uniform<int32_t>(program(), "u_texture");
    }
}
