//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-24.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <vector>
#include "Tungsten/Gpu/GpuTypes.hpp"
#include "SharedBuffer.hpp"
#include "VertexAttribute.hpp"

namespace Tungsten
{
    /**
     * An opaque, backend-owned name for the buffer-and-attribute binding a
     * mesh draws through. On OpenGL it is a VAO id owned by the
     * ResourceManager's cache; a Metal or Vulkan backend would put its own
     * equivalent here, or nothing at all.
     *
     * NONE means "not bound yet" — a mesh created before its stream arenas
     * are known has no binding until they are.
     */
    enum class GeometryBinding : uint32_t
    {
        NONE = 0
    };

    /**
     * A drawable: one or more vertex streams plus an index buffer, described by
     * a VertexLayout. What topology those vertices assemble into is not here —
     * it is pipeline state, because that is where Metal and Vulkan put it.
     *
     * `binding` is computed by ResourceManager::create_mesh and shared by every
     * mesh with the same arena/layout combination; it is not a handle this Mesh
     * owns. The element and buffer bindings are baked into it, the per-draw base
     * offsets (each slice's `offset`) are not, so meshes differing only by
     * offset reuse one binding.
     *
     * Each stream, like `ebo`, is a plain SharedBuffer: its `offset` is the base
     * vertex draws use, its `count` the vertex count, and its byte pitch the
     * owning arena's stride — so there is no separate vertex-stream type.
     *
     * `layout` refers to the interned VertexLayout describing the streams'
     * attributes; resolve it through ResourceManager::get_layout.
     *
     * index_type is the neutral ElementIndexType; it describes the index data
     * in the buffer, so it belongs to the geometry rather than to a pipeline.
     */
    struct Mesh
    {
        GeometryBinding binding = GeometryBinding::NONE;
        std::vector<SharedBuffer> streams;
        VertexLayoutRef layout;
        /**
         * Used for quick validation that provided vertex buffers match the
         * shader's requirements.
         */
        AttributeSemanticMask semantics = 0;
        SharedBuffer ebo;
        ElementIndexType index_type = ElementIndexType::UINT16;
    };
} // Tungsten
