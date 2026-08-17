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
#include "Tungsten/Gl/GlTypes.hpp"
#include "SharedBuffer.hpp"

namespace Tungsten
{
    /**
     * A drawable: one or more vertex streams plus an index buffer, described by a
     * VertexLayout and drawn with a fixed primitive topology.
     *
     * `vao` is the id of a VAO owned and cached by ResourceManager (shared by
     * every mesh with the same arena/layout combination), not a handle this
     * Mesh owns. The element binding and buffer bindings are baked into that VAO;
     * the per-draw base offsets (each slice's `offset`) are not, so meshes
     * differing only by offset reuse one VAO.
     *
     * Each stream, like `ebo`, is a plain SharedBuffer: its `offset` is the base
     * vertex draws use, its `count` the vertex count, and its byte pitch the
     * owning arena's stride — so there is no separate vertex-stream type.
     *
     * `layout` refers to the interned VertexLayout describing the streams'
     * attributes; resolve it through ResourceManager::get_layout.
     *
     * index_type / primitive use the existing GL-layer enums (ElementIndexType,
     * TopologyType) so the renderer can pass them straight to draw_elements
     * without conversion.
     */
    struct Mesh
    {
        uint32_t vao = 0;
        std::vector<SharedBuffer> streams;
        VertexLayoutRef layout;
        SharedBuffer ebo;
        ElementIndexType index_type = ElementIndexType::UINT16;
        TopologyType primitive = TopologyType::TRIANGLES;
    };
} // Tungsten
