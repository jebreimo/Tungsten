//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-26.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/TexturedSmoothMeshShader.hpp"

#include "Tungsten/VertexArrayObjectBuilder.hpp"

namespace Tungsten
{
    VertexArrayObject TexturedSmoothMeshShader::create_vao() const
    {
        return VertexArrayObjectBuilder()
            .add_float(position_attr, 3)
            .add_float(normal_attr, 3)
            .add_float(uv_attr, 2)
            .build();
    }

    void TexturedSmoothMeshShader::set_texture(int32_t texture_handle)
    {
        texture.set(texture_handle);
    }
}
