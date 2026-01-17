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

#include "Viewport.hpp"

namespace Tungsten
{
    enum class ProjectionType
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    struct ProjectionParameters
    {
        float left = -1.0f;
        float right = 1.0f;
        float bottom = -1.0f;
        float top = 1.0f;
        float near = 0.1f;
        float far = 1000.0f;
        ProjectionType type = ProjectionType::PERSPECTIVE;
        bool use_aspect_ratio = false;
    };

    class Camera
    {
    public:
        Camera();

        Camera(const Viewport& viewport,
               const Xyz::Matrix4F& view,
               const ProjectionParameters& projection_parameters);

        [[nodiscard]] const Xyz::Matrix4F& view_matrix() const
        {
            return view_;
        }

        void set_view_matrix(const Xyz::Matrix4F& view);

        [[nodiscard]] Xyz::ViewMatrixComponents<float> decomposed_view_matrix() const;

        [[nodiscard]] const Xyz::Matrix4F& projection_matrix() const
        {
            return projection_;
        }

        [[nodiscard]] const Viewport& viewport() const
        {
            return viewport_;
        }

        void set_viewport(const Viewport& viewport);

        [[nodiscard]] const ProjectionParameters& projection_parameters() const
        {
            return projection_parameters_;
        }

        void set_projection_parameters(const ProjectionParameters& parameters);

    private:
        Viewport viewport_;
        ProjectionParameters projection_parameters_;
        Xyz::Matrix4F view_;
        Xyz::Matrix4F projection_;
    };

    struct CameraBuilder
    {
    public:
        CameraBuilder();

        explicit CameraBuilder(const Camera& camera);

        CameraBuilder& look_at(const Xyz::Vector3F& eye,
                               const Xyz::Vector3F& center,
                               const Xyz::Vector3F& up = {0, 1, 0});

        CameraBuilder& perspective(float fov_y_radians,
                                   float near,
                                   float far);

        CameraBuilder& perspective(float bottom,
                                   float top,
                                   float near,
                                   float far);

        CameraBuilder& perspective(float left,
                                   float right,
                                   float bottom,
                                   float top,
                                   float near,
                                   float far);

        CameraBuilder& orthographic(float bottom,
                                    float top,
                                    float near,
                                    float far);

        CameraBuilder& orthographic(float left,
                                    float right,
                                    float bottom,
                                    float top,
                                    float near,
                                    float far);

        CameraBuilder& viewport(const Viewport& viewport);

        [[nodiscard]] Camera build() const;

    private:
        Xyz::Matrix4F view_;
        Viewport viewport_;
        Xyz::Vector3F position_;
        ProjectionParameters projection_parameters_;
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
