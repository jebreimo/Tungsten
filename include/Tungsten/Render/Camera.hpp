//****************************************************************************
// Copyright © 2025 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2025-12-10.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <string>
#include <Xyz/Matrix.hpp>

#include "../Viewport.hpp"

namespace Tungsten
{
    enum class ProjectionType
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    std::string to_string(ProjectionType type);

    std::ostream& operator<<(std::ostream& stream, ProjectionType type);

    struct ViewParameters
    {
        Xyz::Vector3F position = {0.0f, 0.0f, 0.0f};
        Xyz::Vector3F forward = {0.0f, 0.0f, 1.0f};
        Xyz::Vector3F up = {0.0f, 1.0f, 0.0f};
    };

    std::ostream& operator<<(std::ostream& stream, const ViewParameters& parameters);

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

    std::ostream& operator<<(std::ostream& stream,
                             const ProjectionParameters& parameters);
    class Camera
    {
    public:
        Camera();

        Camera(const Viewport& viewport,
               const ViewParameters& view_parameters,
               const ProjectionParameters& projection_parameters);

        [[nodiscard]] const Xyz::Matrix4F& view_matrix() const
        {
            return view_;
        }

        void set_view_matrix(const Xyz::Matrix4F& view);

        [[nodiscard]] const Xyz::Matrix4F& projection_matrix() const
        {
            return projection_;
        }

        [[nodiscard]] const Viewport& viewport() const
        {
            return viewport_;
        }

        void set_viewport(const Viewport& viewport);

        [[nodiscard]] ViewParameters view_parameters() const;

        void set_view_parameters(const ViewParameters& parameters);

        [[nodiscard]] const ProjectionParameters& projection_parameters() const
        {
            return projection_parameters_;
        }

        void set_projection_parameters(const ProjectionParameters& parameters);

        Xyz::Matrix4F orthographic_screen_matrix();
    private:
        Viewport viewport_;
        ViewParameters view_parameters_;
        ProjectionParameters projection_parameters_;
        Xyz::Matrix4F view_;
        Xyz::Matrix4F projection_;
    };

    std::ostream& operator<<(std::ostream& stream, const Camera& camera);

    /**
     * @brief Calculates the vertical field of view based on the minimum
     *  horizontal and vertical fields of view and the aspect ratio.
     * @param min_fov_x The minimum horizontal field of view in radians.
     * @param min_fov_y The minimum vertical field of view in radians.
     * @param aspect The aspect ratio (width / height).
     * @return The vertical field of view in radians.
     */
    inline float calc_fov_y(float min_fov_x, float min_fov_y, float aspect)
    {
        const auto fov_x = std::max(min_fov_x, min_fov_y * aspect);
        return 2.0f * std::atan(std::tan(fov_x * 0.5f) / aspect);
    }
}
