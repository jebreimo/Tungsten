//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <memory>
#include "Xyz/BBox.hpp"
#include "../Resources/ResourceRefs.hpp"
#include "Skin.hpp"

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
     * A renderable with a skin is drawn deformed by its joints, and its own
     * node's transform is ignored, as glTF prescribes: the joints' world
     * transforms place it. Its material's pipeline must use a skinned shader
     * variant, and its local_bounds should normally be left empty, as the
     * bind-pose bounds say little about where an animated mesh ends up.
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
        /** Null for a rigid mesh. */
        std::shared_ptr<const Skin> skin;
    };
} // Tungsten
