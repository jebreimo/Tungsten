//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>
#include "LightData.hpp"

namespace Tungsten
{
    // Makes a node a light source. Position and direction are not stored
    // here: they come from the owning node's world transform when the
    // SnapshotBuilder extracts the LightData (the direction is the node's
    // -z axis, matching the camera convention).
    struct LightComponent
    {
        LightType type = LightType::POINT;
        Xyz::Vector3F color = {1, 1, 1};
        float intensity = 1.0f;
        float range = 0.0f; // 0 means unbounded
        float inner_cone_angle = 0.0f; // spot lights only
        float outer_cone_angle = 0.0f;
    };
} // Tungsten
