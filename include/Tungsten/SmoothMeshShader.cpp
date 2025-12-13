//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-13.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "SmoothMeshShader.hpp"

#include "VertexArrayObjectBuilder.hpp"

namespace Tungsten
{
    VertexArrayObject SmoothMeshShader::create_vao() const
    {
        return VertexArrayObjectBuilder()
            .add_float(position_attr, 3)
            .add_float(normal_attr, 3)
            .build();
    }

    void SmoothMeshShader::set_material(const Material& material)
    {
        diffuse_albedo.set(material.diffuse_albedo);
        specular_albedo.set(material.specular_albedo);
        specular_power.set(material.specular_exponent);
    }

    void SmoothMeshShader::set_light(const Light& light)
    {
        light_pos.set(light.position);
    }

    void SmoothMeshShader::set_model_view_matrix(const Xyz::Matrix4F& mv)
    {
        mv_matrix.set(mv);
    }

    void SmoothMeshShader::set_projection_matrix(const Xyz::Matrix4F& proj)
    {
        proj_matrix.set(proj);
    }
}
