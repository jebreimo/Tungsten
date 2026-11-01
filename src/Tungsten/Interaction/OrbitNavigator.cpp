//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-01.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../../include/Tungsten/Interaction/OrbitNavigator.hpp"

namespace Tungsten
{
    void OrbitNavigator::orbit(float dx, float dy)
    {
        yaw -= dx * orbit_speed;
        pitch = std::clamp(pitch + dy * orbit_speed,
                            -max_pitch, max_pitch);
    }

    void OrbitNavigator::pan(float dx, float dy)
    {
        const auto step = pan_speed * distance;
        const Xyz::Vector3F right = {std::cos(yaw), 0, -std::sin(yaw)};
        const Xyz::Vector3F forward = {-std::sin(yaw), 0, -std::cos(yaw)};
        target = target - right * (dx * step) + forward * (dy * step);
    }

    void OrbitNavigator::zoom(float delta)
    {
        distance = std::clamp(
            distance * std::pow(zoom_step, -delta),
            min_distance, max_distance);
    }


    Xyz::Vector3F OrbitNavigator::position() const
    {
        const auto x = distance * std::cos(pitch) * std::sin(yaw);
        const auto y = distance * std::sin(pitch);
        const auto z = distance * std::cos(pitch) * std::cos(yaw);
        return target + Xyz::Vector3F{x, y, z};
    }
}
