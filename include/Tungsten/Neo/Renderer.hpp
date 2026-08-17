//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "GlStateCache.hpp"
#include "RenderSnapshot.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    class ResourceManager;

    /**
     * Draws a RenderSnapshot, never touching the scene graph — only the
     * immutable snapshot and the resources its refs name (fetched through
     * the ResourceManager).
     *
     * It owns the three UBO buffer objects of the fixed binding convention.
     * The binding points are set once, in the constructor; afterwards
     * only the buffers' contents change: the per-frame block once per
     * render(), a material's parameter blob when the material changes between
     * items, and the per-draw block per item.
     *
     * Draw order: the items' sort keys are precomputed by the SnapshotBuilder
     * and the renderer only sorts — ascending, and by *pointer* into the
     * snapshot, which therefore stays read-only. The opaque list draws first
     * (front-to-back, batching state changes), then the transparent list with
     * blending enabled (back-to-front).
     */
    class Renderer
    {
    public:
        /**
         * Requires a current GL context: creates the three UBOs and binds
         * them to their fixed binding points.
         */
        explicit Renderer(ResourceManager& resources);

        void render(const RenderSnapshot& snapshot);

    private:
        /**
         * Uploads the per-frame block: camera matrices, camera position and
         * time, ambient light, and up to MAX_LIGHTS lights.
         */
        void bind_per_frame(const RenderSnapshot& snapshot);

        /**
         * Fills sorted_ with pointers to the items, ordered by sort key.
         */
        void sort_items(const std::vector<RenderItem>& items);

        void draw_item(const RenderItem& item);

        /**
         * Selects the item's program, uploads the material's parameter blob
         * to the per-material UBO, and binds its textures to consecutive
         * units. Called only when the material differs from the previous
         * item's, which the sort keeps rare.
         */
        void bind_material(MaterialRef ref);

        ResourceManager& resources_;
        GlStateCache state_;
        BufferHandle per_frame_ubo_;
        BufferHandle per_draw_ubo_;
        /**
         * 1×1 white, bound to every sampler unit the material leaves
         * unfilled: a sampler must see a complete texture even when the
         * shader's runtime flags never read it, and white is the
         * multiplicative identity if it is read anyway.
         */
        TextureHandle white_texture_;
        /**
         * Sort scratch, reused across frames to avoid reallocation.
         */
        std::vector<const RenderItem*> sorted_;
        MaterialRef current_material_;
    };
} // Tungsten
