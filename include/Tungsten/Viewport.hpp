//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-17.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/ProjectionMatrix.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    struct Viewport
    {
        Xyz::Vector2F origin;
        Xyz::Vector2F size;

        [[nodiscard]] float aspect_ratio() const
        {
            return size[1] == 0.0f ? 1.0f : size[0] / size[1];
        }

        [[nodiscard]] Xyz::Matrix4F orthographic_transform() const
        {
            return Xyz::make_orthographic_matrix(origin.x(), origin.x() + size.x(),
                                                 origin.y(), origin.y() + size.y(),
                                                 10000.0f, -10000.0f);
        }
    };

    std::ostream& operator<<(std::ostream& os, const Viewport& viewport);
} // Tungsten
