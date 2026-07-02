//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Gl/GlProgram.hpp"
#include "ResourceRefs.hpp"
#include "ShaderVariantKey.hpp"

namespace Tungsten
{
    // A compiled GL program owned by the ResourceManager and referred to through
    // a ShaderProgramRef (§6). It owns its GL object via the RAII ProgramHandle,
    // so it is move-only — dropping it from its slot deletes the GL program.
    //
    // There is no separate numeric identity: the draw sort key packs the
    // ShaderProgramRef's index (coherent within a snapshot, which is rebuilt
    // every frame), and GlStateCache skips redundant binds by the GL program
    // name (gl_handle.id()).
    //
    // `variant_key` records which family + feature combination this program was
    // compiled for, so the manager can dedupe variants and recompile on demand.
    //
    // `required_layout` refers to the interned vertex format (§12) the program's
    // attributes expect; it is compared against a Mesh's layout ref — interning
    // makes equal layouts share a ref — so mismatches are caught rather than
    // silently mis-binding attributes.
    struct ShaderProgram
    {
        ProgramHandle gl_handle;
        ShaderVariantKey variant_key;
        VertexLayoutRef required_layout;
    };
} // Tungsten