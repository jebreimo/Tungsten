//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include "Tungsten/Gl/GlBuffer.hpp"
#include "PipelineDescriptor.hpp"
#include "ResourceRefs.hpp"

namespace Tungsten
{
    /**
     * The appearance of a drawable: the pipeline that draws it, the parameter
     * values its shader reads, and the textures it samples. Owned by the
     * ResourceManager and referred to through a MaterialRef.
     *
     * `parameter_data` is an opaque byte blob laid out to match the shader's
     * per-material uniform block. ResourceManager uploads it verbatim into
     * this material's own `ubo` at creation, without interpreting it, which is
     * what keeps the renderer shader-agnostic. Drawing the material then only
     * binds that buffer to binding 1 — a material's parameters are uploaded
     * once, not re-uploaded on every switch. Change them afterwards with
     * ResourceManager::update_material_parameters, which is the only way to
     * keep `parameter_data` and the buffer in step.
     *
     * A material with no parameters has an empty `ubo`; that is legal only for
     * a shader that declares no MaterialBlock.
     *
     * `textures` are bound to consecutive sampler units in order. `pipeline`
     * must be registered before the material is used for drawing; it carries
     * the resolved shader variant along with the depth, blend and raster state
     * the draw needs.
     *
     * There is no separate numeric identity: the draw sort key packs the
     * MaterialRef's index (coherent within a snapshot, which is rebuilt every
     * frame), and GlStateCache skips redundant binds by the GL ids of the bound
     * objects (UBO, textures).
     *
     * `queue` is folded from the pipeline by create_material and decides which
     * snapshot list an item lands in: opaque items are sorted for state-change
     * batching (front-to-back), transparent ones back-to-front for correct
     * blending. It is copied here, the way Mesh::semantics is folded from its
     * layout, so extracting a snapshot never has to resolve the pipeline.
     */
    struct Material
    {
        PipelineRef pipeline;
        std::vector<std::byte> parameter_data;
        std::vector<TextureRef> textures;
        RenderQueue queue = RenderQueue::OPAQUE;
        /**
         * The GL buffer holding parameter_data, created by ResourceManager.
         * Owning, so a Material is move-only and dropping it from its slot
         * would delete the buffer — which is why destroy_material retires it
         * through the DeletionQueue instead.
         */
        BufferHandle ubo;
    };
} // Tungsten
