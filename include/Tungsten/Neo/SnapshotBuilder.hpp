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
#include <vector>
#include <Xyz/Matrix.hpp>
#include <Xyz/Vector.hpp>
#include "NodeId.hpp"
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

        /**
         * Fills `out` (typically the scene's back buffer) with what the camera
         * on `camera_node` sees. `out` is cleared first; its storage is reused.
         * The scene-level fields the builder cannot know — time,
         * ambient_light — are left at their defaults for the caller to fill
         * in. Throws if camera_node has no CameraComponent.
         */
        void build(const Scene& scene, NodeId camera_node, RenderSnapshot& out);

    private:
        /**
         * Fills the SoA bounds scratch and the per-item flags for every
         * renderable in the scene. The scratch arrays are sized to the scene's
         * renderable count, which is what the two passes below iterate over.
         */
        void prepare_bounds(const Scene& scene);

        /**
         * Clears visible_ for every item whose bounds fall entirely outside
         * the frustum. Planes outer, items inner: the sign of each plane's
         * normal picks a bounds array once per plane instead of once per item,
         * which leaves the inner loop free of branches.
         */
        void cull();

        /**
         * Extracts renderables from the scene into the RenderSnapshot.
         */
        void extract_renderables(const Scene& scene,
                                 RenderSnapshot& out);

        /**
         * Extracts lights from the scene into the RenderSnapshot.
         */
        static void extract_lights(const Scene& scene, RenderSnapshot& out);

        /**
         * Packs the draw order into one integer, ascending (§14). Opaque:
         * layer, shader, material, mesh, then depth — batching state changes
         * first. Transparent: layer, then *reversed* depth — back-to-front
         * for correct blending — then shader and material.
         */
        [[nodiscard]]
        static uint64_t compute_sort_key(uint32_t layer, uint32_t shader_index,
                                         uint32_t material_index,
                                         uint32_t mesh_index,
                                         float normalized_depth,
                                         bool transparent);

        /**
         * The inverse transpose of the world matrix's upper 3x3, which keeps
         * normals perpendicular under non-uniform scale.
         */
        [[nodiscard]]
        static Xyz::Matrix3F compute_normal_matrix(const Xyz::Matrix4F& world);

        ResourceManager& resources_;

        // Per-build state, set up by build() and read during extraction.
        std::array<Xyz::Vector4F, 6> frustum_planes_;
        Xyz::Matrix4F view_;
        float far_plane_ = 1.0f;

        // Scratch, kept across frames so a steady-state build does not
        // allocate. One array per bounds component, parallel to the scene's
        // renderable array.
        std::vector<float> min_x_, min_y_, min_z_;
        std::vector<float> max_x_, max_y_, max_z_;

        /**
         * Has a mesh, a material, and is visible at all.
         */
        std::vector<uint8_t> drawable_;
        /**
         * Drawable and has bounds — empty bounds mean "unknown", which is never culled.
         */
        std::vector<uint8_t> cullable_;
        /**
         * Survived the frustum test. Meaningless where !cullable_.
         */
        std::vector<uint8_t> visible_;
    };
} // Tungsten
