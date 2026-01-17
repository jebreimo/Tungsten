//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-16.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Camera.hpp"

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
            else // Orthographic
            {
                return Xyz::make_orthographic_matrix(
                    left, right,
                    bottom, top,
                    near, far);
            }
        }
    }

    Camera::Camera()
        : view_{Xyz::make_identity_matrix<float, 4>()},
          projection_{
              make_projection_matrix(projection_parameters_, 1.0f)
          }
    {
    }

    Camera::Camera(const Viewport& viewport, const Xyz::Matrix4F& view,
                   const ProjectionParameters& projection_parameters)
        : viewport_(viewport),
          projection_parameters_(projection_parameters),
          view_(view),
          projection_{
              make_projection_matrix(projection_parameters_,
                                     viewport.aspect_ratio())
          }

    {
    }

    void Camera::set_view_matrix(const Xyz::Matrix4F& view)
    {
        view_ = view;
    }

    Xyz::ViewMatrixComponents<float> Camera::decomposed_view_matrix() const
    {
        return Xyz::decompose_view_matrix(view_);
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

    CameraBuilder::CameraBuilder() = default;

    CameraBuilder::CameraBuilder(const Camera& camera)
        : viewport_(camera.viewport()),
          view_(camera.view_matrix()),
          projection_parameters_(camera.projection_parameters())
    {
    }

    CameraBuilder& CameraBuilder::look_at(const Xyz::Vector3F& eye,
                                          const Xyz::Vector3F& center,
                                          const Xyz::Vector3F& up)
    {
        view_ = Xyz::make_look_at_matrix<float>(eye, center, up);
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
        return {viewport_, view_, projection_parameters_};
    }
} // Tungsten
