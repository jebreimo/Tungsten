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
#include "VertexLayout.hpp"

namespace Tungsten
{
    // Interns the distinct VertexLayouts an application uses and hands them out
    // as VertexLayoutRef (§12). Owned by ResourceManager.
    //
    // Unlike the other resource tables this is deliberately NOT a
    // GenerationalPool: layouts own no GL object and are never individually
    // freed, so there is no free-list, no per-slot generation, and no deletion
    // path. The set of vertex formats is small and bounded, so interning them
    // for the manager's lifetime is cheap and buys a permanently stable ref: a
    // VertexLayoutRef can serve as part of the VaoCache key and be embedded in a
    // sort key without any revocation concern.
    //
    // Because refs never go stale, their generation carries no information; every
    // valid ref uses the fixed LAYOUT_GENERATION and only the index identifies
    // the layout. Generation 0 is left as the null ref, matching the other refs.
    class LayoutRegistry
    {
    public:
        static constexpr uint32_t LAYOUT_GENERATION = 1;

        // Returns the ref for a layout equal to `layout`, interning it first if
        // it has not been seen. Equality is VertexLayout's defaulted operator==
        // (deep-comparing attributes and stride).
        VertexLayoutRef register_layout(const VertexLayout& layout);

        // Resolves a ref to the interned layout, validating it so a null or
        // garbage ref throws rather than aliasing.
        const VertexLayout& get_layout(VertexLayoutRef ref) const;

    private:
        std::vector<VertexLayout> layouts_;
    };
} // Tungsten