//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Camera.hpp"

#include <ostream>
#include <Xyz/ProjectionMatrix.hpp>
#include <Xyz/Utilities.hpp>

namespace Tungsten
{
    namespace
    {
        Xyz::Matrix4F
        make_projection_matrix(const ProjectionParameters& parameters,
                               float aspect)
        {
            auto left = parameters.left;
            auto right = parameters.right;
            const auto bottom = parameters.bottom;
            const auto top = parameters.top;
            const auto near = parameters.near;
            const auto far = parameters.far;

            if (parameters.use_aspect_ratio)
            {
                const auto width = (top - bottom) * aspect;
                left = -width / 2;
                right = width / 2;
            }

            if (parameters.type == ProjectionType::PERSPECTIVE)
            {
                return Xyz::make_frustum_matrix(
                    left, right,
                    bottom, top,
                    near, far);
            }

            // else Orthographic
            return Xyz::make_orthographic_matrix(
                left, right,
                bottom, top,
                near, far);
        }

        ViewParameters normalize(const ViewParameters& parameters)
        {
            const auto forward = Xyz::normalize(parameters.forward);
            return {
                parameters.position,
                forward,
                cross(cross(forward, Xyz::normalize(parameters.up)), forward)
            };
        }

        Xyz::Matrix4F make_view_matrix(const ViewParameters& normalized_params)
        {
            const auto& f = normalized_params.forward;
            const auto& u = normalized_params.up;
            const auto s = Xyz::cross(f, u);
            const auto& p = normalized_params.position;
            return {
                s[0], s[1], s[2], -dot(s, p),
                u[0], u[1], u[2], -dot(u, p),
                -f[0], -f[1], -f[2], dot(f, p),
                0, 0, 0, 1
            };
        }
    }

    std::string to_string(ProjectionType type)
    {
        switch (type)
        {
        case ProjectionType::PERSPECTIVE:
            return "PERSPECTIVE";
        case ProjectionType::ORTHOGRAPHIC:
            return "ORTHOGRAPHIC";
        default:
            return "UNKNOWN";
        }
    }

    std::ostream& operator<<(std::ostream& stream, ProjectionType type)
    {
        stream << to_string(type);
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ViewParameters& parameters)
    {
        stream << "{\n"
               << "  Position: " << parameters.position << "\n"
               << "  Forward: " << parameters.forward << "\n"
               << "  Up: " << parameters.up << "\n"
               << "}";
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ProjectionParameters& parameters)
    {
        stream << "{\n"
               << "  Left: " << parameters.left << ", Right: " << parameters.right << "\n"
               << "  Bottom: " << parameters.bottom << ", Top: " << parameters.top << "\n"
               << "  Near: " << parameters.near << ", Far: " << parameters.far << "\n"
               << "  Type: " << parameters.type
               << "  Use Aspect Ratio: "
               << (parameters.use_aspect_ratio ? "true" : "false") << "\n"
               << "}";
        return stream;
    }

    Camera::Camera()
        : view_{Xyz::make_identity_matrix<float, 4>()},
          projection_{
              make_projection_matrix(projection_parameters_, 1.0f)
          }
    {
    }

    Camera::Camera(const Viewport& viewport,
                   const ViewParameters& view_parameters,
                   const ProjectionParameters& projection_parameters)
        : viewport_(viewport),
          view_parameters_(normalize(view_parameters)),
          projection_parameters_(projection_parameters),
          view_(make_view_matrix(view_parameters_)),
          projection_(make_projection_matrix(projection_parameters_,
                                             viewport_.aspect_ratio()))
    {
    }

    void Camera::set_view_matrix(const Xyz::Matrix4F& view)
    {
        view_ = view;
        const auto& params = Xyz::decompose_view_matrix(view);
        view_parameters_ = normalize({
            params.position,
            params.forward,
            params.up
        });
    }

    ViewParameters Camera::view_parameters() const
    {
        return view_parameters_;
    }

    void Camera::set_view_parameters(const ViewParameters& parameters)
    {
        view_parameters_ = normalize(parameters);
        view_ = make_view_matrix(view_parameters_);
    }

    void Camera::set_viewport(const Viewport& viewport)
    {
        const auto old_aspect = viewport_.aspect_ratio();
        viewport_ = viewport;

        if (projection_parameters_.use_aspect_ratio &&
            old_aspect != viewport_.aspect_ratio())
        {
            projection_ = make_projection_matrix(
                projection_parameters_,
                viewport_.aspect_ratio());
        }
    }

    void Camera::set_projection_parameters(const ProjectionParameters& parameters)
    {
        projection_parameters_ = parameters;
        projection_ = make_projection_matrix(
            projection_parameters_,
            viewport_.aspect_ratio());
    }

    std::ostream& operator<<(std::ostream& stream, const Camera& camera)
    {
        stream << "{\n"
               << "  Viewport: " << camera.viewport() << "\n"
               << "  View Parameters: " << camera.view_parameters() << "\n"
               << "  Projection Parameters: " <<camera.projection_parameters() << "\n"
               << "}\n";
        return stream;
    }

    CameraBuilder::CameraBuilder() = default;

    CameraBuilder::CameraBuilder(const Camera& camera)
        : viewport_(camera.viewport()),
          view_parameters_(camera.view_parameters()),
          projection_parameters_(camera.projection_parameters())
    {
    }

    CameraBuilder& CameraBuilder::look_at(const Xyz::Vector3F& eye,
                                          const Xyz::Vector3F& center,
                                          const Xyz::Vector3F& up)
    {
        view_parameters_ = {eye, center - eye, up};
        return *this;
    }

    CameraBuilder& CameraBuilder::perspective(float fov_y_radians, float near, float far)
    {
        const auto t = near * std::tan(fov_y_radians / 2);
        return perspective(-t, t, near, far);
    }

    CameraBuilder& CameraBuilder::perspective(float bottom, float top, float near, float far)
    {
        projection_parameters_ = {
            -1.0f, 1.0f,
            bottom, top,
            near, far,
            ProjectionType::PERSPECTIVE,
            true
        };
        return *this;
    }

    CameraBuilder& CameraBuilder::perspective(float left, float right, float bottom, float top,
                                              float near, float far)
    {
        projection_parameters_ = {
            left, right,
            bottom, top,
            near, far,
            ProjectionType::PERSPECTIVE,
            false
        };
        return *this;
    }

    CameraBuilder& CameraBuilder::orthographic(float bottom, float top, float near, float far)
    {
        projection_parameters_ = {
            -1.0f, 1.0f,
            bottom, top,
            near, far,
            ProjectionType::ORTHOGRAPHIC,
            true
        };
        return *this;
    }

    CameraBuilder& CameraBuilder::orthographic(float left, float right, float bottom, float top,
                                               float near, float far)
    {
        projection_parameters_ = {
            left, right,
            bottom, top,
            near, far,
            ProjectionType::ORTHOGRAPHIC,
            false
        };
        return *this;
    }

    CameraBuilder& CameraBuilder::viewport(const Viewport& viewport)
    {
        viewport_ = viewport;
        return *this;
    }

    Camera CameraBuilder::build() const
    {
        return {viewport_, view_parameters_, projection_parameters_};
    }
} // Tungsten
