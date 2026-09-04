//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include "../SceneGraph/NodeId.hpp"
#include "RenderSnapshot.hpp"

namespace Tungsten
{
    class ResourceManager;
    class Scene;

    /**
     * Extracts an immutable RenderSnapshot from the mutable scene graph — the
     * bridge between the two sides. Call Scene::resolve_transforms first so
     * extraction reads finalized world matrices.
     *
     * Extraction is three flat passes over the scene's renderable array rather
     * than a walk of the hierarchy: world-space bounds into struct-of-arrays
     * scratch, then a frustum test over those arrays, then RenderItems for
     * whatever survived. Splitting it that way is what makes the middle pass —
     * the one that touches every object every frame — a branch-free loop over
     * six float arrays that a compiler can vectorize.
     *
     * The builder resolves MaterialRefs through the ResourceManager to reach
     * the shader ref and transparency for the sort key.
     */
    class SnapshotBuilder
    {
    public:
        explicit SnapshotBuilder(ResourceManager& resources);

        ~SnapshotBuilder();

        /**
         * The builder holds a ref into one ResourceManager, so it is neither
         * copied nor moved.
         */
        SnapshotBuilder(const SnapshotBuilder&) = delete;
        SnapshotBuilder& operator=(const SnapshotBuilder&) = delete;

        /**
         * Fills `out` (typically the scene's back buffer) with what the camera
         * on `camera_node` sees. `out` is cleared first; its storage is reused.
         * The scene-level fields the builder cannot know — time,
         * ambient_light — are left at their defaults for the caller to fill
         * in. Throws if camera_node has no CameraComponent.
         */
        void build(const Scene& scene, NodeId camera_node, RenderSnapshot& out);

    private:
        struct Members;
        std::unique_ptr<Members> members_;
    };
} // Tungsten
