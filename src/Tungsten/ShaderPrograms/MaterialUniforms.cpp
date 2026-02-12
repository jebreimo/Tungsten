//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-02-12.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/ShaderPrograms/MaterialUniforms.hpp"
#include "Tungsten/ShaderPrograms/ShaderProgram.hpp"

namespace Tungsten
{
    ColorMaterialUniform::ColorMaterialUniform(const ShaderProgram& shader_program)
        : ambient(get_uniform<Xyz::Vector3F>(shader_program.handle(), "u_material.ambient")),
          diffuse(get_uniform<Xyz::Vector3F>(shader_program.handle(), "u_material.diffuse")),
          specular(get_uniform<Xyz::Vector3F>(shader_program.handle(), "u_material.specular")),
          shininess(get_uniform<float>(shader_program.handle(), "u_material.shininess"))
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
