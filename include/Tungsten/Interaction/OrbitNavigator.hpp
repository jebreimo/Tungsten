//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    class OrbitNavigator
    {
    public:
        Xyz::Vector3F target = {0, 0, 0};
        float yaw = 0.5f;
        float pitch = 0.35f;
        float distance = 8.0f;

        float orbit_speed = 0.005f;
        float pan_speed = 0.002f;
        float zoom_step = 1.1f;
        float min_distance = 0.5f;
        float max_distance = 500.0f;
        float max_pitch = 1.55f;

        void orbit(float dx, float dy);
        void pan(float dx, float dy);
        void zoom(float delta);
        [[nodiscard]] Xyz::Vector3F position() const;
    };
}
