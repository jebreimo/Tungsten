//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-27.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <array>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    enum class LightType
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    class LightData
    {
    public:
        [[nodiscard]] LightType type() const;

        void set_type(LightType type);

        [[nodiscard]] Xyz::Vector3F position() const;

        void set_position(const Xyz::Vector3F& position);

        [[nodiscard]] Xyz::Vector3F direction() const;

        void set_direction(const Xyz::Vector3F& direction);

        [[nodiscard]] Xyz::Vector3F color() const;

        void set_color(const Xyz::Vector3F& color);

        [[nodiscard]] float intensity() const;

        void set_intensity(float intensity);

        [[nodiscard]] float range() const;

        void set_range(float range);

        [[nodiscard]] float inner_cone_angle() const;

        void set_inner_cone_angle(float angle);

        [[nodiscard]] float outer_cone_angle() const;

        void set_outer_cone_angle(float angle);

        [[nodiscard]] const std::array<float, 32>& data() const;
    private:
        std::array<float, 32> data_ = {};
    };
} // Tungsten
