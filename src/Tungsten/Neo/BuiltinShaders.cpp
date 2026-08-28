//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/BuiltinShaders.hpp"

#include "Tungsten/Neo/ResourceManager.hpp"
#include "Shaders/BuiltinShaderSources.hpp"
#
namespace Tungsten
{
    void register_builtin_shader_families(ResourceManager& resources)
    {
        ShaderFamily blinn_phong;
        blinn_phong.vertex_source = BLINN_PHONG_VERTEX;
        blinn_phong.fragment_source = BLINN_PHONG_FRAGMENT;
        blinn_phong.samplers = {"u_diffuse_map", "u_specular_map"};
        blinn_phong.required_attributes =
            semantic_bit(AttributeSemantic::POSITION)
            | semantic_bit(AttributeSemantic::NORMAL)
            | semantic_bit(AttributeSemantic::TEX_COORD_0);
        resources.register_shader_family(BLINN_PHONG_FAMILY,
                                         std::move(blinn_phong));
    }
} // Tungsten
