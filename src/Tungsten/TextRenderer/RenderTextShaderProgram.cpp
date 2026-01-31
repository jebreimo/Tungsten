//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-09-01.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "RenderTextShaderProgram.hpp"
#include "Tungsten/ShaderProgramBuilder.hpp"
#include "Tungsten/VertexArrayObjectBuilder.hpp"
#include "../ShaderSources.hpp"

namespace Tungsten::Detail
{
    RenderTextShaderProgram::RenderTextShaderProgram()
        : ShaderProgram("internal::Text",
                        {
                            {ShaderType::VERTEX, ShaderSources::RENDER_TEXT_VERTEX},
                            {ShaderType::FRAGMENT, ShaderSources::RENDER_TEXT_FRAGMENT}
                        })
    {
        position = get_vertex_attribute(handle(), "a_Position");
        texture_coord = get_vertex_attribute(handle(), "a_TextureCoord");

        mvp_matrix = Tungsten::get_uniform<Xyz::Matrix4F>(handle(), "u_MvpMatrix");
        texture = Tungsten::get_uniform<int32_t>(handle(), "u_Texture");
        color = Tungsten::get_uniform<Xyz::Vector4F>(handle(), "u_TextColor");
    }

    VertexArrayObject RenderTextShaderProgram::create_vao(int32_t extra_stride) const
    {
        return VertexArrayObjectBuilder()
            .add_float(position, 2)
            .add_float(texture_coord, 2)
            .add_stride(extra_stride)
            .build();
    }
}
