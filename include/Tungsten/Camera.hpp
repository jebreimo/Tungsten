//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Matrix.hpp>
#include <Xyz/ProjectionMatrix.hpp>

namespace Tungsten
{
    struct Camera
    {
        Xyz::Matrix4F view;
        Xyz::Matrix4F projection;
    };

    /**
     * @brief Calculates the vertical field of view based on the minimum
     *  horizontal and vertical fields of view and the aspect ratio.
     * @param min_fov_x The minimum horizontal field of view in radians.
     * @param min_fov_y The minimum vertical field of view in radians.
     * @param aspect The aspect ratio (width / height).
     * @return The vertical field of view in radians.
     */
    inline float calculate_fov_y(float min_fov_x, float min_fov_y, float aspect)
    {
        const auto fov_x = std::max(min_fov_x, min_fov_y * aspect);
        return 2.0f * std::atan(std::tan(fov_x * 0.5f) / aspect);
    }
}
