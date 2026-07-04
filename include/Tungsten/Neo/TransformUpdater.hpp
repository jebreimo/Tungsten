//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once

namespace Tungsten
{
    class Node;
    class Scene;

    // Finalizes the scene's derived data once per frame, before the
    // SnapshotBuilder extracts it (§2, §5). One traversal does both
    // directions:
    //
    // - Transforms propagate *down*: visiting every node's world_matrix()
    //   resolves the dirty subtrees; unchanged nodes cost one version
    //   comparison, so this is not an eager full-tree recompute.
    // - Bounds propagate *up*: after a node's children are resolved, its
    //   world bounds is rebuilt as its own renderables' world-space bounds
    //   unioned with the children's bounds, so a moved child grows its
    //   ancestors' bounds.
    class TransformUpdater
    {
    public:
        static void resolve(const Scene& scene);

    private:
        static void resolve_node(const Node& node);

        static void propagate_bounds(const Node& node);
    };
} // Tungsten
