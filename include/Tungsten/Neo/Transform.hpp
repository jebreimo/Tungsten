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
#include <Xyz/TransformationMatrix.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    // The local transform of a Node: a plain value with no cache and no dirty
    // flag of its own (§2). Node::set_local_transform is the only mutation
    // path and owns the one dirty flag, so there is no way to change a node's
    // transform that bypasses it.
    //
    // local_matrix() composes translation * rotation * scale on demand; a TRS
    // compose is cheap and happens at most once per world-matrix recompute.
    //
    // rotation is Xyz's yaw/pitch/roll Orientation (Xyz has no quaternion
    // type). A 2D scene rotates about the z axis only, i.e. sets just yaw
    // (§8).
    struct Transform
    {
        Xyz::Vector3F translation = {0, 0, 0};
        Xyz::Orientation3F rotation{0, 0, 0};
        Xyz::Vector3F scale = {1, 1, 1};

        [[nodiscard]]
        Xyz::Matrix4F local_matrix() const
        {
            // Assembles T * R * S directly from the 3x3 rotation: applying
            // the scale to the rotation's columns is R * S, and T only fills
            // the last column. (Xyz::affine::to_matrix(orientation, offset)
            // would do T * R in one call, but its offset parameter is
            // declared with std::type_identity<T> instead of
            // std::type_identity_t<T>, which makes it uncallable.)
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
