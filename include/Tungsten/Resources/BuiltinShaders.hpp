//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ShaderVariantKey.hpp"

namespace Tungsten
{
    class ResourceManager;

    /**
     * The builtin Blinn-Phong shader family, which is registered by
     * register_builtin_shader_families. It is a simple, untextured Blinn-Phong
     * shader with a single directional light.
     */
    inline constexpr ShaderFamilyId BLINN_PHONG_FAMILY = 1;

    /**
     * The builtin text shader family: unlit, textured quads whose colour comes
     * from the material's parameter block.
     *
     * Unlike BlinnPhong it is *not* registered by
     * register_builtin_shader_families — TextSystem's constructor registers it,
     * so the family and the code that compiles meshes for it have one owner and
     * there is no ordering requirement between them. The id lives here because
     * this is where the family id space is documented.
     */
    inline constexpr ShaderFamilyId TEXT_FAMILY = 2;

    /**
     * The first shader family id that applications may use for their own families.
     */
    inline constexpr ShaderFamilyId FIRST_USER_SHADER_FAMILY = 1000;

    /**
     * Registers the builtin shader families (currently BlinnPhong, from the
     * GLSL sources embedded from the Shaders directory) with the manager,
     * interning the layout they require. Variants are still compiled lazily,
     * by register_shader_variant (§14).
     */
    void register_builtin_shader_families(ResourceManager& resources);
} // Tungsten
