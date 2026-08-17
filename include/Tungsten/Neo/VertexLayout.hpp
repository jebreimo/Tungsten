//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-29.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "VertexAttribute.hpp"

namespace Tungsten
{
    /**
     * The vertex format a Mesh advertises and a ShaderProgram requires: the set
     * of attributes (each tagged with its semantic and source stream) plus the
     * byte stride of a vertex.
     *
     * Layouts are interned by ResourceManager and handed out as VertexLayoutRef:
     * a small comparable value identifies the format and forms part of
     * the VAO cache key without the cache needing the full layout. Interning and
     * the cache key both rely on value equality, hence the defaulted operator==.
     *
     * A stream's byte pitch is its arena's stride (one arena per stride);
     * this stride describes the common (single, interleaved) stream case and is
     * what the caller uses to pick or create the right arena.
     */
    struct VertexLayout
    {
        std::vector<VertexAttribute> attributes;
        uint16_t stride = 0;

        bool operator==(const VertexLayout&) const = default;
    };
} // Tungsten
