//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <optional>
#include <Xyz/Vector.hpp>
#include "../Resources/ResourceRefs.hpp"
#include "../Viewport.hpp"

namespace Tungsten
{
    /**
     * What happens to an attachment's existing contents when a pass begins.
     */
    enum class LoadAction
    {
        /** Keep what is there and draw over it. */
        LOAD,
        /** Overwrite it with the attachment's clear value. */
        CLEAR,
        /**
         * The pass writes every pixel, so the old contents need not be
         * fetched. A hint: a tiler can skip loading the tile.
         */
        DONT_CARE
    };

    /**
     * What happens to an attachment's contents when a pass ends.
     */
    enum class StoreAction
    {
        /** Keep the result; something will read it. */
        STORE,
        /**
         * Nothing reads it afterwards, so it need not be written back. On a
         * tiler — which is most of the WebGL2 audience — discarding a depth
         * buffer this way saves real bandwidth.
         */
        DONT_CARE
    };

    struct ColorAttachment
    {
        LoadAction load = LoadAction::CLEAR;
        StoreAction store = StoreAction::STORE;
        /**
         * Written verbatim, never encoded — like every colour that does not
         * reach the framebuffer through a fragment shader, it is given in
         * sRGB, the space the rest of the library authors colours in.
         */
        Xyz::Vector4F clear_color = {0, 0, 0, 1};
    };

    struct DepthAttachment
    {
        LoadAction load = LoadAction::CLEAR;
        StoreAction store = StoreAction::DONT_CARE;
        float clear_depth = 1.0f;
    };

    /**
     * A rectangle outside which nothing is drawn, in pixels.
     */
    struct ScissorRect
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t width = 0;
        int32_t height = 0;
    };

    /**
     * Everything about *where* a snapshot is drawn and what happens to that
     * surface around the draw: the target, the region of it being drawn to,
     * and each attachment's load and store behaviour.
     *
     * This is the counterpart to PipelineDescriptor, and the split follows
     * Metal and Vulkan: per-draw fixed-function state belongs to the pipeline,
     * while the render target, viewport and scissor belong to the pass. Load
     * and store actions have no OpenGL equivalent as objects — they become a
     * glClear and a glInvalidateFramebuffer — but they are what a tiler needs
     * and what both target APIs require, so they are stated here rather than
     * left implicit.
     *
     * A null `target` means the window's framebuffer.
     *
     * Note this is a *single*-pass signature. Something like a shadow map
     * needs either a filter on which items a pass draws or an explicit
     * begin/end pair; neither exists yet, so do not read this as final.
     */
    struct RenderPassDescriptor
    {
        RenderTargetRef target;
        Viewport viewport;
        std::optional<ScissorRect> scissor;
        ColorAttachment color;
        std::optional<DepthAttachment> depth = DepthAttachment{};
    };
} // Tungsten
