//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include "Xyz/BBox.hpp"
#include "../Resources/ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * Makes a node drawable: which mesh to draw with which material, the
     * mesh's bounds in the node's local space (used for frustum culling), and
     * where the item sorts (render_layer forms the top bits of the snapshot
     * sort key).
     *
     * Leaving local_bounds empty means "no bounds known": the item is never
     * culled.
     *
     * A plain aggregate with no base class and no back-pointer to its node —
     * the scene stores these in one flat array per kind and keeps the owning
     * NodeId alongside.
     */
    struct RenderableComponent
    {
        MeshRef mesh;
        MaterialRef material;
        Xyz::BBox3F local_bounds;
        bool visible = true;
        uint32_t render_layer = 0;
    };
} // Tungsten
