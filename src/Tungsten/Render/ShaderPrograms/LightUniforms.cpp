//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/ShaderPrograms/LightUniforms.hpp"

#include "Tungsten/Render/ShaderPrograms/ShaderProgram.hpp"

namespace Tungsten
{
    DirectionalLightUniform::DirectionalLightUniform(const std::string& prefix,
                                                     const ShaderProgram& shader_program)
        : direction(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "direction")),
          ambient(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "ambient")),
          diffuse(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "diffuse")),
          specular(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "specular"))
    {
    }

    void DirectionalLightUniform::set(const DirectionalLight& light,
                                      const Xyz::Matrix4F& view_matrix)
    {
        direction.set(make_submatrix<3, 3>(view_matrix, {}) * light.direction);
        ambient.set(light.light.ambient);
        diffuse.set(light.light.diffuse);
        specular.set(light.light.specular);
    }
}
