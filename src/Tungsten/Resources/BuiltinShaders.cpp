//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/BuiltinShaders.hpp"

#include "Tungsten/Resources/ResourceManager.hpp"
#include "Shaders/BuiltinShaderSources.hpp"
#
namespace Tungsten
{
    void register_builtin_shader_families(ResourceManager& resources)
    {
        ShaderFamily blinn_phong;
        blinn_phong.vertex_source = BLINN_PHONG_VERTEX;
        blinn_phong.fragment_source = BLINN_PHONG_FRAGMENT;
        // The specular map is a reflectance mask and the normal map a vector
        // field: neither is colour, so neither is sRGB-decoded on sampling.
        blinn_phong.samplers = {
            {"u_diffuse_map", TextureContent::COLOR},
            {"u_specular_map", TextureContent::DATA},
            {"u_normal_map", TextureContent::DATA}
        };
        blinn_phong.required_attributes =
            semantic_bit(AttributeSemantic::POSITION)
            | semantic_bit(AttributeSemantic::NORMAL)
            | semantic_bit(AttributeSemantic::TEX_COORD_0);
        resources.register_shader_family(BLINN_PHONG_FAMILY,
                                         std::move(blinn_phong));
    }
} // Tungsten
