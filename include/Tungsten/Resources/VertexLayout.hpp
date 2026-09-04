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
     * How a Mesh's vertices are packed: which attributes exist, and where each
     * one sits — which stream it is read from and at what offset within that
     * stream's vertex.
     *
     * Layouts are interned by ResourceManager and handed out as VertexLayoutRef:
     * a small comparable value identifies the packing and forms part of
     * the VAO cache key without the cache needing the full layout. Interning and
     * the cache key both rely on value equality, hence the defaulted operator==.
     */
    struct VertexLayout
    {
        std::vector<VertexAttribute> attributes;

        bool operator==(const VertexLayout&) const = default;

        /**
         * The set of semantics this layout provides, with the packing dropped.
         * Cheap to fold, and cached on a Mesh at creation so the per-draw check
         * against a shader's requirement costs no lookup.
         */
        [[nodiscard]]
        AttributeSemanticMask semantics() const
        {
            AttributeSemanticMask mask = 0;
            for (const VertexAttribute& attribute : attributes)
                mask = AttributeSemanticMask(mask | semantic_bit(attribute.semantic));
            return mask;
        }
    };
} // Tungsten
