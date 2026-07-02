//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "ResourceRefs.hpp"

namespace Tungsten
{
    // A non-owning slice of a BufferArena's GL buffer. The arena owns the
    // single BufferHandle; this just names a sub-range of it, so it is a plain
    // copyable value (no refcount, no ownership). Resolve the arena to its
    // BufferHandle through ResourceManager::get_arena().
    //
    // offset and count are in the arena's stride units (vertices or indices),
    // not bytes (§7): offset is directly the base vertex / first index a draw
    // uses and count the vertex/index count. This is why a Mesh needs no
    // separate vertex-stream type — each of its streams, and its index slice,
    // is just a SharedBuffer.
    struct SharedBuffer
    {
        BufferArenaRef arena;
        uint32_t offset;
        uint32_t count;
    };
}
