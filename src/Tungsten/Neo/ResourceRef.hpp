//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>

namespace Tungsten
{
    // A logical, generational reference to a resource owned by the
    // ResourceManager. Distinct from the GL/RAII *Handle types (BufferHandle,
    // ProgramHandle, ...), which own a GL id. The template parameter is a
    // phantom tag: it carries no data but stops a ref to one resource type from
    // being passed where another is expected.
    //
    // The generation is bumped when a slot is freed, so a stale ref (held
    // across a deferred deletion + slot reuse) fails validation on lookup
    // instead of silently aliasing a different resource. A ref is therefore
    // revocable and must not be persisted as if it were a stable id.
    template <typename T>
    struct ResourceRef
    {
        uint32_t index;
        uint32_t generation;

        bool operator==(const ResourceRef&) const = default;
    };
}