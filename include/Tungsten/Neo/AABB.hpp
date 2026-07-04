//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <algorithm>
#include <limits>
#include <Xyz/Matrix.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    // An axis-aligned bounding box. The default is the *empty* box
    // (min > max), which is the identity of merge(): merging anything with an
    // empty box yields the other operand. Empty also means "no bounds known";
    // the SnapshotBuilder never culls an item with empty bounds.
    struct AABB
    {
        Xyz::Vector3F min = {
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        };
        Xyz::Vector3F max = {
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()
        };

        [[nodiscard]]
        bool is_empty() const
        {
            return min[0] > max[0] || min[1] > max[1] || min[2] > max[2];
        }
    };

    [[nodiscard]]
    inline AABB merge(const AABB& a, const AABB& b)
    {
        AABB result;
        for (unsigned i = 0; i < 3; ++i)
        {
            result.min[i] = std::min(a.min[i], b.min[i]);
            result.max[i] = std::max(a.max[i], b.max[i]);
        }
        return result;
    }

    [[nodiscard]]
    inline AABB merge(const AABB& box, const Xyz::Vector3F& point)
    {
        AABB result = box;
        for (unsigned i = 0; i < 3; ++i)
        {
            result.min[i] = std::min(result.min[i], point[i]);
            result.max[i] = std::max(result.max[i], point[i]);
        }
        return result;
    }

    // The axis-aligned box enclosing `box` under the affine transform `m`
    // (whose last row must be 0 0 0 1, which holds for any TRS world matrix):
    // transforms all eight corners and wraps them. Empty stays empty.
    [[nodiscard]]
    inline AABB transformed(const AABB& box, const Xyz::Matrix4F& m)
    {
        if (box.is_empty())
            return {};

        AABB result;
        for (unsigned i = 0; i < 8; ++i)
        {
            const Xyz::Vector3F corner = {
                (i & 1) ? box.max[0] : box.min[0],
                (i & 2) ? box.max[1] : box.min[1],
                (i & 4) ? box.max[2] : box.min[2]
            };
            Xyz::Vector3F point;
            for (unsigned row = 0; row < 3; ++row)
            {
                point[row] = m[row, 0] * corner[0]
                             + m[row, 1] * corner[1]
                             + m[row, 2] * corner[2]
                             + m[row, 3];
            }
            result = merge(result, point);
        }
        return result;
    }
} // Tungsten
