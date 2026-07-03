//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "CameraComponent.hpp"

#include <Xyz/InvertMatrix.hpp>
#include <Xyz/ProjectionMatrix.hpp>

#include "Node.hpp"

namespace Tungsten
{
    Xyz::Matrix4F CameraComponent::get_view_matrix() const
    {
        if (const Node* node = owner())
            return Xyz::invert(node->world_matrix());
        return Xyz::Matrix4F::identity();
    }

    Xyz::Matrix4F CameraComponent::get_projection_matrix() const
    {
        if (mode == ProjectionMode::ORTHOGRAPHIC)
        {
            const float half_height = ortho_size;
            const float half_width = ortho_size * aspect;
            return Xyz::make_orthographic_matrix(
                -half_width, half_width,
                -half_height, half_height,
                near_plane, far_plane);
        }
        return Xyz::make_perspective_matrix(fov, aspect, near_plane, far_plane);
    }
} // Tungsten
