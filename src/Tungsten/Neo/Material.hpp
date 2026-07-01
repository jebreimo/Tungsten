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
    // The appearance of a drawable: which shader draws it, the parameter values
    // that shader reads, and the textures it samples. Owned by the
    // ResourceManager and referred to through a MaterialRef (§6).
    //
    // `parameter_data` is an opaque byte blob laid out to match the shader's
    // per-material uniform block; the renderer uploads it verbatim to the
    // per-material UBO (binding 1, §4) on material change without interpreting
    // it, which is what keeps the renderer shader-agnostic. The typed *Uniform
    // helpers may be used to construct this blob.
    //
    // `textures` are bound to consecutive sampler units in order. `shader` must
    // be resolved to a concrete variant before the material is used for drawing.
    //
    // `id` is a stable numeric identity used by the renderer as part of the draw
    // sort key (so items sharing a material batch together and only trigger one
    // per-material bind) and by GlStateCache to skip redundant binds. It is
    // distinct from the MaterialRef, which is a revocable generational handle.
    struct Material
    {
        ShaderProgramRef shader;
        std::vector<uint8_t> parameter_data;
        std::vector<TextureRef> textures;
        uint32_t id = 0;
    };
} // Tungsten