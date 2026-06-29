//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "SharedBuffer.hpp"

namespace Tungsten
{
    // One vertex-buffer binding of a Mesh: a sub-range of an arena's GL buffer
    // (vbo) holding vertex_count vertices of `stride` bytes each, the first at
    // vertex index first_vertex within that buffer. A VertexLayout's attributes
    // address into this stream via their stream_index and offset_in_stream.
    //
    // first_vertex is the per-stream base offset that array draws pass as
    // `first` and indexed draws fold into rebased indices (§3, §7), so it is not
    // baked into the shared VAO.
    struct VertexStream
    {
        SharedBuffer vbo;
        uint32_t first_vertex = 0;
        uint16_t stride = 0;
        uint32_t vertex_count = 0;
    };
} // Tungsten