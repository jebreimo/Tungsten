//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "TransformUpdater.hpp"

#include "RenderableComponent.hpp"
#include "Scene.hpp"

namespace Tungsten
{
    void TransformUpdater::resolve(const Scene& scene)
    {
        for (const auto& root : scene.roots())
            resolve_node(*root);
    }

    void TransformUpdater::resolve_node(const Node& node)
    {
        // Touching the world matrix resolves it (and any stale ancestors)
        // lazily; a clean node costs one version comparison.
        (void)node.world_matrix();

        for (const auto& child : node.children())
            resolve_node(*child);

        // Children are resolved, so their bounds are final: this is the
        // upward half of the pass.
        propagate_bounds(node);
    }

    void TransformUpdater::propagate_bounds(const Node& node)
    {
        AABB bounds;
        for (const auto& component : node.components())
        {
            const auto* renderable =
                dynamic_cast<const RenderableComponent*>(component.get());
            if (renderable && renderable->visible)
            {
                bounds = merge(bounds, transformed(renderable->local_bounds,
                                                   node.world_matrix()));
            }
        }
        for (const auto& child : node.children())
            bounds = merge(bounds, child->world_bounds());

        node.world_bounds_ = bounds;
    }
} // Tungsten
