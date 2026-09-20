//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-20.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include "Tungsten/Gpu/GpuTypes.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * How a draw reads and writes the depth buffer.
     *
     * The test and the write mask are separate because a blended pass wants
     * the test on and the mask off: transparent surfaces stay occluded by
     * opaque geometry without occluding each other.
     */
    struct DepthState
    {
        bool test = true;
        bool write = true;
        CompareFunction compare = CompareFunction::LESS;

        bool operator==(const DepthState&) const = default;
    };

    /**
     * How a draw's output is combined with what is already in the target.
     *
     * The colour and alpha factors are separate so a premultiplied or additive
     * blend can be spelled; set_blend_function's two-argument form is the
     * common case where they agree.
     */
    struct BlendState
    {
        bool enabled = false;
        BlendFunction src_color = BlendFunction::ONE;
        BlendFunction dst_color = BlendFunction::ZERO;
        BlendFunction src_alpha = BlendFunction::ONE;
        BlendFunction dst_alpha = BlendFunction::ZERO;
        BlendEquation color_equation = BlendEquation::ADD;
        BlendEquation alpha_equation = BlendEquation::ADD;
        ColorWriteMask color_write = ColorWriteMask::ALL;

        bool operator==(const BlendState&) const = default;
    };

    /**
     * How triangles are turned into fragments.
     */
    struct RasterState
    {
        bool cull_enabled = true;
        FaceCullingMode cull_mode = FaceCullingMode::BACK;
        FrontFace front_face = FrontFace::COUNTER_CLOCKWISE;

        bool operator==(const RasterState&) const = default;
    };

    /**
     * Which list of the snapshot a draw belongs to, and therefore how it is
     * sorted.
     *
     * Deliberately not derived from BlendState::enabled: the two are different
     * questions. Additive particles blend but are order-independent, and
     * alpha-tested foliage does not blend but may still want sorting.
     */
    enum class RenderQueue
    {
        /** Sorted front-to-back, to batch state changes and reject early. */
        OPAQUE,
        /** Sorted back-to-front, so blending composites in the right order. */
        TRANSPARENT
    };

    /**
     * Everything about a draw that is fixed before the draw call: the program,
     * the vertex input it expects, the topology it assembles, and the
     * fixed-function state it needs.
     *
     * This is Tungsten's stand-in for a Metal MTLRenderPipelineState or a
     * Vulkan VkPipeline. It is immutable and interned — register it once,
     * refer to it by PipelineRef — so that switching appearance is a value
     * comparison rather than a sequence of state toggles, and so a backend
     * that really does bake pipelines has something to bake.
     *
     * What is NOT here is as deliberate as what is. The viewport, the scissor
     * rectangle and the render target belong to the render pass, because that
     * is where Metal and Vulkan put them; the per-draw transform and the
     * material's parameters are buffer contents, not pipeline state.
     *
     * `layout` is the vertex input the shader is compiled against. A mesh may
     * only be drawn through a pipeline whose layout it matches exactly — a
     * shader constrains which attribute semantics must be present, but a
     * pipeline pins how they are packed, which is what MTLVertexDescriptor
     * and VkPipelineVertexInputStateCreateInfo describe.
     */
    struct PipelineDescriptor
    {
        ShaderProgramRef shader;
        VertexLayoutRef layout;
        TopologyType primitive = TopologyType::TRIANGLES;
        RenderQueue queue = RenderQueue::OPAQUE;
        DepthState depth;
        BlendState blend;
        RasterState raster;

        bool operator==(const PipelineDescriptor&) const = default;
    };
} // Tungsten
