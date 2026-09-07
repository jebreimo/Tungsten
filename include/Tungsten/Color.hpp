//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cmath>
#include <Xyz/Vector.hpp>

/**
 * @file
 * Conversions between the sRGB encoding colours are authored in and the linear
 * space lighting arithmetic has to be done in.
 *
 * The rule the library follows: every colour handed to Tungsten by name — a
 * material's reflectance, a light's colour, a text style, the clear colour —
 * is sRGB, because that is what a colour picker shows and what a texture file
 * holds. Everything the GPU computes with is linear. Colours from textures are
 * decoded by the sampler (see TextureContent); colours passed as values are
 * converted by the functions here, at the point they cross into a uniform
 * buffer.
 */

namespace Tungsten
{
    /**
     * Decodes one sRGB-encoded channel to linear.
     *
     * This is the exact piecewise sRGB curve rather than a 2.2 power
     * approximation, so it agrees with the hardware decode applied to sRGB
     * textures — a constant and a texture naming the same colour must produce
     * the same value, or untextured and textured surfaces will not match.
     */
    [[nodiscard]] inline float srgb_to_linear(float value)
    {
        if (value <= 0.04045f)
            return value / 12.92f;
        return std::pow((value + 0.055f) / 1.055f, 2.4f);
    }

    /** Encodes one linear channel to sRGB. */
    [[nodiscard]] inline float linear_to_srgb(float value)
    {
        if (value <= 0.0031308f)
            return value * 12.92f;
        return 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
    }

    [[nodiscard]] inline Xyz::Vector3F srgb_to_linear(const Xyz::Vector3F& color)
    {
        return {srgb_to_linear(color[0]),
                srgb_to_linear(color[1]),
                srgb_to_linear(color[2])};
    }

    [[nodiscard]] inline Xyz::Vector3F linear_to_srgb(const Xyz::Vector3F& color)
    {
        return {linear_to_srgb(color[0]),
                linear_to_srgb(color[1]),
                linear_to_srgb(color[2])};
    }

    /** Alpha is a coverage fraction, not a colour, so it is left alone. */
    [[nodiscard]] inline Xyz::Vector4F srgb_to_linear(const Xyz::Vector4F& color)
    {
        return {srgb_to_linear(color[0]),
                srgb_to_linear(color[1]),
                srgb_to_linear(color[2]),
                color[3]};
    }

    [[nodiscard]] inline Xyz::Vector4F linear_to_srgb(const Xyz::Vector4F& color)
    {
        return {linear_to_srgb(color[0]),
                linear_to_srgb(color[1]),
                linear_to_srgb(color[2]),
                color[3]};
    }
} // Tungsten
