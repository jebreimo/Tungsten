//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-03.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Constants.hpp>
#include <Xyz/Matrix.hpp>

namespace Tungsten
{
    enum class ProjectionMode
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    // Makes a node a camera. The one branch between 3D and 2D (§8): mode
    // selects a perspective projection (uses fov) or an orthographic one
    // (uses ortho_size). Position and viewing direction come from the owning
    // node's world transform — the camera looks along its node's -z axis.
    //
    // The near and far planes are named near_plane / far_plane because `near`
    // and `far` are macros in Windows headers. `aspect` (width / height) is
    // viewport state the application updates on resize.
    struct CameraComponent
    {
        ProjectionMode mode = ProjectionMode::PERSPECTIVE;
        float fov = Xyz::Constants<float>::PI / 3; // vertical, in radians
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
        float ortho_size = 1.0f; // half the height of the view volume
        float aspect = 1.0f;

        // The view matrix for a camera on a node with the given world
        // transform, i.e. that matrix inverted. Static because it depends on
        // nothing else in the component; it stays a member to keep the two
        // matrices that make up a camera together.
        [[nodiscard]]
        static Xyz::Matrix4F get_view_matrix(
            const Xyz::Matrix4F& node_world_matrix);

        [[nodiscard]]
        Xyz::Matrix4F get_projection_matrix() const;
    };
} // Tungsten
