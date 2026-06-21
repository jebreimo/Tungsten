//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Render/ShaderPrograms/MaterialUniforms.hpp"
#include "Tungsten/Render/ShaderPrograms/ShaderProgram.hpp"

namespace Tungsten
{
    ColorMaterialUniform::ColorMaterialUniform(const std::string& prefix,
                                               const ShaderProgram& shader_program)
        : ambient(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "ambient")),
          diffuse(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "diffuse")),
          specular(get_uniform<Xyz::Vector3F>(shader_program.id(), prefix + "specular")),
          shininess(get_uniform<float>(shader_program.id(), prefix + "shininess"))
    {
    }

    void ColorMaterialUniform::set(const ColorMaterial& material)
    {
        ambient.set(material.ambient);
        diffuse.set(material.diffuse);
        specular.set(material.specular);
        shininess.set(material.shininess);
    }
}
