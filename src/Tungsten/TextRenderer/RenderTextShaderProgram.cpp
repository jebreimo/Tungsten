//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "RenderTextShaderProgram.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "RenderText-frag.glsl.hpp"
#include "RenderText-vert.glsl.hpp"

namespace Tungsten::Detail
{
    RenderTextShaderProgram::RenderTextShaderProgram()
    {
        using namespace Tungsten;
        program = ShaderProgramBuilder()
            .add_shader(ShaderType::VERTEX, RenderText_vert)
            .add_shader(ShaderType::FRAGMENT, RenderText_frag)
            .build();

        position = get_vertex_attribute(program, "a_Position");
        texture_coord = get_vertex_attribute(program, "a_TextureCoord");

        mvp_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(program, "u_MvpMatrix");
        texture = Tungsten::get_uniform<int32_t>(program, "u_Texture");
        color = Tungsten::get_uniform<Xyz::Vector4F>(program, "u_TextColor");
    }
}
