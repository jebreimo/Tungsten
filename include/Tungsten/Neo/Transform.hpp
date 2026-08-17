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
    /**
     * Returns the rotation with the given yaw, pitch and roll, for callers
     * that think in Euler angles. The quaternion is the canonical form; this
     * is a convenience for setting Transform::rotation.
     */
    [[nodiscard]]
    inline Xyz::QuaternionF euler_rotation(float yaw, float pitch, float roll)
    {
        return Xyz::to_quaternion(Xyz::Orientation3F(yaw, pitch, roll));
    }

    /**
     * Returns the rotation of @a angle radians about the z axis, i.e. the
     * only rotation a 2D scene needs.
     */
    [[nodiscard]]
    inline Xyz::QuaternionF z_rotation(float angle)
    {
        return Xyz::make_quaternion<float>(angle, {0, 0, 1});
    }

    /**
     * The rotation that maps the vehicle-style frame — +x longitudinal
     * (forward), +y lateral (left), +z up — onto the scene graph's frame,
     * where forward is -z, up is +y and right is +x. A 120 degree turn
     * about (-1, 1, 1); every component is exactly one half.
     */
    [[nodiscard]]
    constexpr Xyz::QuaternionF x_forward_to_scene_rotation()
    {
        return {0.5f, -0.5f, 0.5f, 0.5f};
    }

    /**
     * Returns @a rotation, an orientation expressed in the +x forward,
     * +y left, +z up frame, expressed in the scene graph's frame instead.
     * Both the reference and the rotated axes are rebased, hence the
     * conjugation rather than a plain multiplication.
     */
    [[nodiscard]]
    inline Xyz::QuaternionF x_forward_to_scene(const Xyz::QuaternionF& rotation)
    {
        const auto r = x_forward_to_scene_rotation();
        return r * rotation * Xyz::conjugate(r);
    }

    /**
     * Returns the rotation that makes a node at @a position face @a target,
     * i.e. the rotation that aims the scene graph's forward axis (-z) at the
     * target while keeping the node's up axis (+y) as close to @a up as
     * possible. For a camera, @a target is the point at the center of the
     * frame.
     *
     * Throws if @a position and @a target coincide, or if the line between
     * them is parallel to @a up.
     */
    [[nodiscard]]
    inline Xyz::QuaternionF look_at_rotation(const Xyz::Vector3F& position,
                                             const Xyz::Vector3F& target,
                                             const Xyz::Vector3F& up = {0, 1, 0})
    {
        const auto forward = target - position;
        const auto right = Xyz::cross(forward, up);
        // The arguments are the node's right (+x) and up (+y) axes. The
        // latter must be the up that is perpendicular to forward, not @a up
        // itself, as to_quaternion makes +z their cross product — and +z has
        // to end up opposite forward.
        return Xyz::to_quaternion(right, Xyz::cross(right, forward));
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
