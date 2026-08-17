//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-06-27.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Neo/LightData.hpp"

#include <cmath>

namespace Tungsten
{
    // The type travels in position.w, which the shader reads as a float and
    // *converts* — `int type = int(light.position.w)` in BlinnPhong-frag.glsl.
    // So it has to be stored as the float value 0, 1 or 2, not as the enum's
    // bit pattern: reinterpreting POINT's bits as a float gives 1.4e-45, which
    // converts back to 0 and turns every point light into a directional one.
    LightType LightData::type() const
    {
        return static_cast<LightType>(static_cast<int>(data_[3]));
    }

    void LightData::set_type(LightType type)
    {
        data_[3] = static_cast<float>(static_cast<int>(type));
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

    // The setters take angles but the blob stores their cosines: the shaders
    // compare Light.cone directly against dot products
    // (BlinnPhong-frag.glsl), and packing the wire format is exactly what the
    // accessors are for (like set_type above).

    float LightData::inner_cone_angle() const
    {
        return std::acos(data_[12]);
    }

    void LightData::set_inner_cone_angle(float angle)
    {
        data_[12] = std::cos(angle);
    }

    float LightData::outer_cone_angle() const
    {
        return std::acos(data_[13]);
    }

    void LightData::set_outer_cone_angle(float angle)
    {
        data_[13] = std::cos(angle);
    }

    const std::array<float, LIGHT_DATA_SIZE>& LightData::data() const
    {
        return data_;
    }
} // Tungsten
