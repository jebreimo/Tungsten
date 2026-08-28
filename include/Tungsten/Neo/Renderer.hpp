//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstddef>
#include <vector>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "Tungsten/Gl/GlTexture.hpp"
#include "GlStateCache.hpp"
#include "RenderSnapshot.hpp"
#include "ResourceRefs.hpp"
#include "VertexAttribute.hpp"

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
         * Appends pointers to the items to sorted_ and orders that run by sort
         * key, leaving anything already there untouched.
         */
        void append_sorted(const std::vector<RenderItem>& items);

        /**
         * Packs every sorted item's per-draw block into one buffer, spaced by
         * per_draw_stride_, and uploads it. An item's index in sorted_ is its
         * slot, so draw_item can bind its slice without a lookup.
         */
        void upload_per_draw_blocks();

        /**
         * Draws one item, binding its slice of the packed per-draw buffer.
         */
        void draw_item(const RenderItem& item, size_t slot);

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
         * Sort scratch, reused across frames to avoid reallocation. Holds the
         * opaque run first, then the transparent one; an item's index here is
         * its slot in the per-draw buffer.
         */
        std::vector<const RenderItem*> sorted_;
        /**
         * The packed per-draw blocks, staged here before the one upload.
         */
        std::vector<std::byte> staging_;
        /**
         * Spacing between per-draw blocks: the block size rounded up to
         * GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, which every glBindBufferRange
         * offset has to be a multiple of.
         */
        size_t per_draw_stride_ = 0;
        MaterialRef current_material_;
        /**
         * The required_attributes of current_material_'s shader, cached so the
         * per-draw check needs no lookup. Cleared with the material.
         */
        AttributeSemanticMask current_required_attributes_ = 0;
        /**
         * The sampler bound to units the current material leaves unfilled.
         * Re-resolved at the start of each frame.
         */
        uint32_t default_sampler_id_ = 0;
    };
} // Tungsten
