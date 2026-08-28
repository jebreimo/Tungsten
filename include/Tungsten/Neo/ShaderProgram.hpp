//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Tungsten/Gl/GlProgram.hpp"
#include "ResourceRefs.hpp"
#include "VertexAttribute.hpp"
#include "ShaderVariantKey.hpp"

namespace Tungsten
{
    /**
     * A compiled GL program owned by the ResourceManager and referred to through
     * a ShaderProgramRef. It owns its GL object via the RAII ProgramHandle,
     * so it is move-only — dropping it from its slot deletes the GL program.
     *
     * The draw sort key packs the ShaderProgramRef's index (coherent within
     * a snapshot, which is rebuilt every frame), and GlStateCache skips
     * redundant binds by the GL program name (gl_handle.id()).
     */
    struct ShaderProgram
    {
        ProgramHandle gl_handle;
        /**
         * Records which family + feature combination this program was
         * compiled for, so the manager can dedupe variants and recompile on demand.
         */
        ShaderVariantKey variant_key;
        /**
         * The semantics the program's attributes read, copied from its family.
         * Renderer::draw_item tests it against the mesh's own cached set, so a
         * mesh that does not supply something its shader reads throws rather
         * than drawing from an attribute that was never enabled.
         */
        AttributeSemanticMask required_attributes = 0;
        /**
         * The size of the family's sampler list: the program
         * samples texture units [0, sampler_count), so the renderer knows which
         * units must hold a texture even when the material provides none.
         */
        uint32_t sampler_count = 0;
        /**
         * Records whether the linked program actually declares
         * the MaterialBlock of the binding convention. The renderer uses it to
         * catch a material whose parameter_data is empty although its shader reads
         * the block — which would otherwise draw with whatever the previous
         * material left in the UBO.
         */
        bool has_material_block = false;
    };
} // Tungsten
