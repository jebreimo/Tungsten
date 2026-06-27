//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-27.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "LightData.hpp"

namespace Tungsten
{
    LightType LightData::type() const
    {
        const auto type = *reinterpret_cast<const uint32_t*>(&data_[3]);
        return static_cast<LightType>(type);
    }

    void LightData::set_type(LightType type)
    {
        const auto value = static_cast<uint32_t>(type);
        *reinterpret_cast<uint32_t*>(&data_[3]) = value;
    }

    Xyz::Vector3F LightData::position() const
    {
        return {data_[0], data_[1], data_[2]};
    }

    void LightData::set_position(const Xyz::Vector3F& position)
    {
        data_[0] = position[0];
        data_[1] = position[1];
        data_[2] = position[2];
    }

    Xyz::Vector3F LightData::direction() const
    {
        return {data_[4], data_[5], data_[6]};
    }

    void LightData::set_direction(const Xyz::Vector3F& direction)
    {
        data_[4] = direction[0];
        data_[5] = direction[1];
        data_[6] = direction[2];
    }

    Xyz::Vector3F LightData::color() const
    {
        return {data_[8], data_[9], data_[10]};
    }

    void LightData::set_color(const Xyz::Vector3F& color)
    {
        data_[8] = color[0];
        data_[9] = color[1];
        data_[10] = color[2];
    }

    float LightData::intensity() const
    {
        return data_[11];
    }

    void LightData::set_intensity(float intensity)
    {
        data_[11] = intensity;
    }

    float LightData::range() const
    {
        return data_[7];
    }

    void LightData::set_range(float range)
    {
        data_[7] = range;
    }

    float LightData::inner_cone_angle() const
    {
        return data_[12];
    }

    void LightData::set_inner_cone_angle(float angle)
    {
        data_[12] = angle;
    }

    float LightData::outer_cone_angle() const
    {
        return data_[13];
    }

    void LightData::set_outer_cone_angle(float angle)
    {
        data_[13] = angle;
    }

    const std::array<float, 32>& LightData::data() const
    {
        return data_;
    }
} // Tungsten
