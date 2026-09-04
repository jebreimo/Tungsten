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

    /**
     * Makes a node a camera. Position and viewing direction come from the
     * owning node's world transform — the camera looks along its node's -z axis.
     */
    struct CameraComponent
    {
        /**
         * The projection mode. Perspective uses fov, orthographic uses
         * ortho_size.
         */
        ProjectionMode mode = ProjectionMode::PERSPECTIVE;
        /**
         * The vertical field of view, in radians. Only used in perspective
         * mode.
         */
        float fov = Xyz::Constants<float>::PI / 3;
        /**
         * The distance to the near plane. Must be positive.
         */
        float near_plane = 0.1f;
        /**
         * The distance to the far plane. Must be positive.
         */
        float far_plane = 1000.0f;
        /**
         * The half-height of the orthographic view volume. Only used in
         * orthographic mode.
         */
        float ortho_size = 1.0f;
        /**
         * The aspect ratio (width / height) of the viewport.
         */
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
