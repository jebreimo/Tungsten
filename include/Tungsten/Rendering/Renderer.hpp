//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <memory>
#include "RenderPassDescriptor.hpp"
#include "RenderSnapshot.hpp"

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
     * (front-to-back, batching state changes), then the transparent list
     * (back-to-front). Both draw in one loop: each item's depth, blend and
     * raster state travels on its pipeline rather than being toggled per pass.
     */
    class Renderer
    {
    public:
        /**
         * Requires a current GL context: creates the three UBOs and binds
         * them to their fixed binding points.
         */
        explicit Renderer(ResourceManager& resources);

        ~Renderer();

        /**
         * The renderer holds a ref into one ResourceManager and owns the GL
         * objects bound to the fixed binding points, so it is neither copied
         * nor moved.
         */
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /**
         * Draws @a snapshot into the pass's target.
         *
         * The pass says where to draw and what to do with the surface around
         * the draw — clearing it, the viewport, an optional scissor — so an
         * application never touches global GL state to set those up.
         */
        void render(const RenderSnapshot& snapshot,
                    const RenderPassDescriptor& pass);

    private:
        struct Members;
        std::unique_ptr<Members> members_;
    };
} // Tungsten
