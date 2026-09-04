//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-04.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <span>

#include "Tungsten/Gl/GlVertexArray.hpp"
#include "VertexLayout.hpp"

namespace Tungsten
{
    /**
     * Creates a vertex array that reads @a layout's attributes from
     * @a buffer_ids, an attribute's stream_index selecting the buffer it is
     * read from. Each stream's stride is derived from the attributes that
     * name it, so their vertices are treated as tightly packed.
     *
     * Attribute locations follow the AttributeSemantic convention — POSITION
     * at 0, NORMAL at 1, and so on — so every shader drawn with the returned
     * vertex array has to declare its attributes at those locations.
     *
     * Throws if an attribute names a stream that @a buffer_ids has no buffer
     * for, or if any of the buffer ids is zero.
     */
    [[nodiscard]]
    VertexArrayHandle make_vertex_array(std::span<const uint32_t> buffer_ids,
                                        const VertexLayout& layout);

    /**
     * Creates a vertex array reading every attribute from one buffer.
     */
    [[nodiscard]]
    VertexArrayHandle make_vertex_array(uint32_t buffer_id,
                                        const VertexLayout& layout);
} // Tungsten
