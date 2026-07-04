//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ShaderVariantKey.hpp"
#include "VertexLayout.hpp"

namespace Tungsten
{
    class ResourceManager;

    // The family ids used by register_builtin_shader_families. Applications
    // registering their own families should pick ids above
    // FIRST_USER_SHADER_FAMILY.
    inline constexpr ShaderFamilyId BLINN_PHONG_FAMILY = 1;
    inline constexpr ShaderFamilyId FIRST_USER_SHADER_FAMILY = 1000;

    // The interleaved position / normal / texcoord vertex format the builtin
    // families expect: three streams' worth of attributes in one 32-byte
    // stream, at the fixed AttributeSemantic locations (§13).
    [[nodiscard]]
    VertexLayout builtin_pnt_layout();

    // Registers the builtin shader families (currently BlinnPhong, from the
    // embedded Shaders/*.glsl sources) with the manager, interning the layout
    // they require. Variants are still compiled lazily, by
    // register_shader_variant (§14).
    void register_builtin_shader_families(ResourceManager& resources);
} // Tungsten
