//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Tungsten/Neo/AABB.hpp"
#include "Component.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    // Makes a Node drawable: which mesh to draw with which material, the
    // mesh's bounds in the node's local space (used for frustum culling and
    // for the aggregate bounds TransformUpdater propagates), and where the
    // item sorts (render_layer forms the top bits of the snapshot sort key).
    //
    // Leaving local_bounds empty means "no bounds known": the item is never
    // culled, and it contributes nothing to the aggregate bounds.
    struct RenderableComponent : Component
    {
        MeshRef mesh;
        MaterialRef material;
        AABB local_bounds;
        bool visible = true;
        uint32_t render_layer = 0;
    };
} // Tungsten
