//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>

namespace Tungsten
{
    // Interned identity of a shader family (e.g. "PBR", "unlit"). Callers think
    // in terms of families and feature flags rather than concrete compiled
    // programs; the ResourceManager maps a family name to this small comparable
    // id when a family is registered.
    using ShaderFamilyId = uint32_t;

    // The lazy-compilation key for a shader variant (§5 of the design chat).
    // Rather than register one ShaderProgram per feature combination — which
    // explodes in count — a Material selects a family plus a bitmask of enabled
    // features (HAS_NORMAL_MAP, HAS_SKINNING, ALPHA_CLIP, ...). ResourceManager's
    // register_shader_variant() compiles-or-fetches the matching program and
    // hands back a ShaderProgramRef.
    //
    // Equality is value-based (defaulted operator==) so the key can index the
    // variant cache: two keys with the same family and defines resolve to the
    // same compiled program.
    struct ShaderVariantKey
    {
        ShaderFamilyId family = 0;
        uint32_t defines = 0;

        bool operator==(const ShaderVariantKey&) const = default;
    };
} // Tungsten