//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Matrix.hpp>
#include <Xyz/Orientation.hpp>
#include <Xyz/Quaternion.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    // Returns the rotation with the given yaw, pitch and roll, for callers
    // that think in Euler angles. The quaternion is the canonical form; this
    // is a convenience for setting Transform::rotation.
    [[nodiscard]]
    inline Xyz::QuaternionF euler_rotation(float yaw, float pitch, float roll)
    {
        return Xyz::to_quaternion(Xyz::Orientation3F(yaw, pitch, roll));
    }

    // Returns the rotation of @a angle radians about the z axis, i.e. the
    // only rotation a 2D scene needs (§8).
    [[nodiscard]]
    inline Xyz::QuaternionF z_rotation(float angle)
    {
        return Xyz::make_quaternion<float>(angle, {0, 0, 1});
    }

    /**
     * The local transform of a Node.
     */
    struct Transform
    {
        Xyz::Vector3F translation = {0, 0, 0};
        Xyz::QuaternionF rotation;
        Xyz::Vector3F scale = {1, 1, 1};

        [[nodiscard]]
        Xyz::Matrix4F make_matrix() const
        {
            // Assembles T * R * S directly from the 3x3 rotation.
            const auto r = Xyz::linear::to_matrix(rotation);
            return {
                r[0, 0] * scale[0], r[0, 1] * scale[1], r[0, 2] * scale[2], translation[0],
                r[1, 0] * scale[0], r[1, 1] * scale[1], r[1, 2] * scale[2], translation[1],
                r[2, 0] * scale[0], r[2, 1] * scale[1], r[2, 2] * scale[2], translation[2],
                0, 0, 0, 1
            };
        }
    };
} // Tungsten
