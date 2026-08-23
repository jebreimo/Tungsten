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

namespace Tungsten
{
    VertexLayout builtin_pnt_layout()
    {
        VertexLayout layout;
        layout.attributes = {
            {AttributeSemantic::POSITION, 0,
             VertexAttributeDataType::FLOAT, 3, false, 0},
            {AttributeSemantic::NORMAL, 0,
             VertexAttributeDataType::FLOAT, 3, false, 12},
            {AttributeSemantic::TEX_COORD_0, 0,
             VertexAttributeDataType::FLOAT, 2, false, 24},
        };
        layout.stride = 32;
        return layout;
    }

    VertexLayout text_vertex_layout()
    {
        VertexLayout layout;
        layout.attributes = {
            {AttributeSemantic::POSITION, 0,
             VertexAttributeDataType::FLOAT, 2, false, 0},
            {AttributeSemantic::TEX_COORD_0, 0,
             VertexAttributeDataType::FLOAT, 2, false, 8},
        };
        layout.stride = 16;
        return layout;
    }

    void register_builtin_shader_families(ResourceManager& resources)
    {
        ShaderFamily blinn_phong;
        blinn_phong.vertex_source = BLINN_PHONG_VERTEX;
        blinn_phong.fragment_source = BLINN_PHONG_FRAGMENT;
        // No compile-time features yet: the maps and light counts are
        // runtime-switched uniforms (§14 prefers runtime control).
        // Sampler i samples texture unit i, so a material's textures list
        // uses this order: diffuse map first, then specular map.
        blinn_phong.samplers = {"u_diffuse_map", "u_specular_map"};
        blinn_phong.required_layout =
            resources.register_layout(builtin_pnt_layout());
        resources.register_shader_family(BLINN_PHONG_FAMILY,
                                         std::move(blinn_phong));
    }
} // Tungsten
