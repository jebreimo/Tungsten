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

    // The local transform of a Node: a plain value with no cache and no dirty
    // flag of its own (§2). Node::set_local_transform is the only mutation
    // path and owns the one dirty flag, so there is no way to change a node's
    // transform that bypasses it.
    //
    // local_matrix() composes translation * rotation * scale on demand; a TRS
    // compose is cheap and happens at most once per world-matrix recompute.
    //
    // rotation is a quaternion rather than Euler angles: quaternions compose
    // and interpolate, they have no gimbal lock, and building a rotation
    // matrix from one costs no trigonometry at all. Use euler_rotation() or
    // z_rotation() to construct one from angles. Accumulated products need no
    // renormalizing for rendering — Xyz's to_matrix divides by the squared
    // length, so a drifted quaternion still yields a proper rotation rather
    // than a scaled one.
    struct Transform
    {
        Xyz::Vector3F translation = {0, 0, 0};
        Xyz::QuaternionF rotation;
        Xyz::Vector3F scale = {1, 1, 1};

        [[nodiscard]]
        Xyz::Matrix4F local_matrix() const
        {
            // Assembles T * R * S directly from the 3x3 rotation: applying
            // the scale to the rotation's columns is R * S, and T only fills
            // the last column. (Xyz::affine::to_matrix(rotation, offset)
            // would do T * R in one call, but there is no way to fold the
            // scale into it, so it would cost an extra matrix product.)
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
