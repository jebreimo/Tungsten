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
#include "VertexLayout.hpp"
#include "VertexStream.hpp"

namespace Tungsten
{
    // A drawable: one or more vertex streams plus an index buffer, described by a
    // VertexLayout and drawn with a fixed primitive topology.
    //
    // `vao` is the id of a VAO owned and cached by ResourceManager (shared by
    // every mesh with the same arena/layout combination, §7), not a handle this
    // Mesh owns. The element binding and buffer bindings are baked into that VAO;
    // the per-draw offsets (first_index, and each stream's first_vertex) are not,
    // so meshes differing only by offset reuse one VAO.
    //
    // index_type / primitive use the existing GL-layer enums (ElementIndexType,
    // TopologyType) so the renderer can pass them straight to draw_elements
    // without conversion.
    struct Mesh
    {
        uint32_t vao = 0;
        std::vector<VertexStream> streams;
        VertexLayout layout;
        SharedBuffer ebo;
        uint32_t first_index = 0;
        uint32_t index_count = 0;
        ElementIndexType index_type = ElementIndexType::UINT16;
        TopologyType primitive = TopologyType::TRIANGLES;
    };
} // Tungsten