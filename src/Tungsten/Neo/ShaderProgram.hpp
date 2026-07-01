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
#include "ShaderVariantKey.hpp"
#include "VertexLayout.hpp"

namespace Tungsten
{
    // A compiled GL program owned by the ResourceManager and referred to through
    // a ShaderProgramRef (§6). It owns its GL object via the RAII ProgramHandle,
    // so it is move-only — dropping it from its slot deletes the GL program.
    //
    // `id` is a stable numeric identity for the compiled variant, used by the
    // renderer as part of the draw sort key (so items sharing a program batch
    // together and only trigger one use_program) and by GlStateCache to skip
    // redundant binds. It is distinct from the ShaderProgramRef, which is a
    // revocable generational handle into the manager's slot table.
    //
    // `variant_key` records which family + feature combination this program was
    // compiled for, so the manager can dedupe variants and recompile on demand.
    //
    // `required_layout` is the vertex format the program's attributes expect;
    // the renderer matches it against a Mesh's VertexLayout when building the
    // VAO so mismatches are caught rather than silently mis-binding attributes.
    struct ShaderProgram
    {
        ProgramHandle gl_handle;
        uint32_t id = 0;
        ShaderVariantKey variant_key;
        VertexLayout required_layout;
    };
} // Tungsten