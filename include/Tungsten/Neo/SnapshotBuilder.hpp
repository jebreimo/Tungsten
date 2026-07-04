//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <array>
#include <cstdint>
#include <Xyz/Matrix.hpp>
#include <Xyz/Vector.hpp>
#include "AABB.hpp"
#include "RenderSnapshot.hpp"

namespace Tungsten
{
    struct CameraComponent;
    struct LightComponent;
    class Node;
    struct RenderableComponent;
    class ResourceManager;
    class Scene;

    // Extracts an immutable RenderSnapshot from the mutable scene graph — the
    // bridge between the two sides (§1, §5). Call TransformUpdater::resolve
    // first so extraction reads finalized world matrices and bounds.
    //
    // build() walks the scene once: renderables are frustum-culled against
    // their world-space bounds, split into the opaque and transparent lists
    // by their material, and given a precomputed sort key (the renderer only
    // sorts); lights become LightData with position and direction taken from
    // their node's world transform.
    //
    // The builder resolves MaterialRefs through the ResourceManager to reach
    // the shader ref and transparency for the sort key — the "resolves
    // handles" dependency in the diagram.
    class SnapshotBuilder
    {
    public:
        explicit SnapshotBuilder(ResourceManager& resources);

        // Fills `out` (typically the scene's back buffer). `out` is cleared
        // first; its storage is reused (§5). The scene-level fields the
        // builder cannot know — time, ambient_light — are left at their
        // defaults for the caller to fill in.
        void build(const Scene& scene, const CameraComponent& camera,
                   RenderSnapshot& out);

    private:
        void extract_node(const Node& node, RenderSnapshot& out);

        void extract_renderable(const Node& node,
                                const RenderableComponent& renderable,
                                RenderSnapshot& out);

        static LightData extract_light(const Node& node,
                                       const LightComponent& light);

        // True if the box is entirely outside the view frustum. Empty bounds
        // mean "no bounds known" and are never culled.
        [[nodiscard]]
        bool cull(const AABB& world_bounds) const;

        // Packs the draw order into one integer, ascending (§14). Opaque:
        // layer, shader, material, mesh, then depth — batching state changes
        // first. Transparent: layer, then *reversed* depth — back-to-front
        // for correct blending — then shader and material.
        [[nodiscard]]
        static uint64_t compute_sort_key(uint32_t layer, uint32_t shader_index,
                                         uint32_t material_index,
                                         uint32_t mesh_index,
                                         float normalized_depth,
                                         bool transparent);

        // The inverse transpose of the world matrix's upper 3x3, which keeps
        // normals perpendicular under non-uniform scale.
        [[nodiscard]]
        static Xyz::Matrix3F compute_normal_matrix(const Xyz::Matrix4F& world);

        ResourceManager& resources_;

        // Per-build state, set up by build() and read during the traversal.
        std::array<Xyz::Vector4F, 6> frustum_planes_;
        Xyz::Matrix4F view_;
        float far_plane_ = 1.0f;
    };
} // Tungsten
