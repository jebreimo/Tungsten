//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-28.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>

namespace Tungsten
{
    // Fixed UBO binding points, shared by the renderer, the GlStateCache, and
    // every builtin shader (§4 of docs/scene_graph_design.md). They are fixed
    // across all shaders so the renderer never has to branch on shader type, and
    // must stay in sync with the `layout(std140) ... binding` of the builtin
    // shaders (e.g. PerFrame / PerDraw in src/Tungsten/Neo/Shaders/*.glsl).
    inline constexpr uint32_t PER_FRAME_UBO_BINDING = 0;    // camera, lights, time
    inline constexpr uint32_t PER_MATERIAL_UBO_BINDING = 1; // material parameters
    inline constexpr uint32_t PER_DRAW_UBO_BINDING = 2;     // model / normal matrix
} // Tungsten