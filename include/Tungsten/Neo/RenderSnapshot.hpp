//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-27.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
#include <Xyz/Matrix.hpp>
#include <Xyz/Vector.hpp>

#include "LightData.hpp"
#include "RenderItem.hpp"

namespace Tungsten
{
    // The immutable hand-off between the scene graph and the renderer (§1, §5).
    // SnapshotBuilder fills the back buffer of Scene's DoubleBuffer; after the
    // swap the renderer reads the front buffer and never observes half-built
    // state. "Immutable" is by convention (the renderer only reads it), not by
    // const members: the same instances are reused across frames, so the fields
    // are plain and reassignable.
    //
    // Items are split into the two passes the renderer draws in order: opaque
    // first (front-to-back, sorted by sort_key for state-change batching), then
    // transparent (back-to-front for correct blending).
    //
    // The scalar/light fields below are the CPU-side mirror of the per-frame
    // UBO (binding 0, §4) the renderer uploads once per frame; the trailing
    // comments map each to its u_* slot in the shaders' PerFrame block.
    struct RenderSnapshot
    {
        std::vector<RenderItem> opaque_items;
        std::vector<RenderItem> transparent_items;
        Xyz::Matrix4F view_matrix;        // u_view
        Xyz::Matrix4F projection_matrix;  // u_projection
        Xyz::Vector3F camera_position;    // u_camera_pos.xyz (world space)
        Xyz::Vector3F ambient_light;      // u_ambient_light.rgb
        std::vector<LightData> lights;    // u_lights / u_light_count
        float time = 0.0f;                // u_camera_pos.w

        // Resets the snapshot for reuse, keeping the vectors' backing storage so
        // rebuilding it next frame does not reallocate (§5).
        void clear()
        {
            opaque_items.clear();
            transparent_items.clear();
            lights.clear();
            view_matrix = {};
            projection_matrix = {};
            camera_position = {};
            ambient_light = {};
            time = 0.0f;
        }
    };
} // Tungsten
