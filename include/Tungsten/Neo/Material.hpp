//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * The appearance of a drawable: which shader draws it, the parameter values
     * that shader reads, and the textures it samples. Owned by the
     * ResourceManager and referred to through a MaterialRef.
     *
     * `parameter_data` is an opaque byte blob laid out to match the shader's
     * per-material uniform block; the renderer uploads it verbatim to the
     * per-material UBO (binding 1) on material change without interpreting
     * it, which is what keeps the renderer shader-agnostic.
     *
     * `textures` are bound to consecutive sampler units in order. `shader` must
     * be resolved to a concrete variant before the material is used for drawing.
     *
     * There is no separate numeric identity: the draw sort key packs the
     * MaterialRef's index (coherent within a snapshot, which is rebuilt every
     * frame), and GlStateCache skips redundant binds by the GL ids of the bound
     * objects (UBO, textures).
     * `transparent` decides which snapshot list an item lands in: opaque
     * items are sorted for state-change batching (front-to-back), transparent
     * ones back-to-front for correct blending.
     */
    struct Material
    {
        ShaderProgramRef shader;
        std::vector<std::byte> parameter_data;
        std::vector<TextureRef> textures;
        bool transparent = false;
    };
} // Tungsten
