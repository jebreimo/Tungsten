//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-07.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Color.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Tungsten/Resources/Texture.hpp"
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;
using Catch::Matchers::WithinAbs;

TEST_CASE("sRGB: black and white survive the conversion exactly")
{
    // The renderer's 1x1 white fallback is bound to colour and data slots
    // alike, which is only sound because decoding leaves these two values
    // where they were.
    REQUIRE(srgb_to_linear(0.0f) == 0.0f);
    REQUIRE(srgb_to_linear(1.0f) == 1.0f);

    // The encode direction is exact at zero but lands one ulp under one:
    // 1.055 - 0.055 is not quite 1 in single precision. It quantises to 255
    // all the same, so it is a tolerance rather than a hole.
    REQUIRE(linear_to_srgb(0.0f) == 0.0f);
    REQUIRE_THAT(linear_to_srgb(1.0f), WithinAbs(1.0, 1e-6));
}

TEST_CASE("sRGB: the two directions are inverses")
{
    for (float value : {0.0f, 0.002f, 0.04f, 0.25f, 0.5f, 0.75f, 1.0f})
    {
        REQUIRE_THAT(linear_to_srgb(srgb_to_linear(value)),
                     WithinAbs(value, 1e-6));
    }
}

TEST_CASE("sRGB: mid grey decodes darker than it is encoded")
{
    // The whole point of the exercise: 50% grey on screen is about 21% of the
    // light, and shading has to work with the latter.
    REQUIRE_THAT(srgb_to_linear(0.5f), WithinAbs(0.21404f, 1e-5));
}

TEST_CASE("sRGB: the curve is continuous across the linear segment's end")
{
    // The piecewise curve's two halves must meet, or colours near black would
    // jump depending on which branch produced them.
    constexpr float cutoff = 0.04045f;
    REQUIRE_THAT(srgb_to_linear(cutoff - 1e-6f),
                 WithinAbs(srgb_to_linear(cutoff + 1e-6f), 1e-6));
}

TEST_CASE("sRGB: alpha is left alone")
{
    const auto color = srgb_to_linear(Xyz::Vector4F{0.5f, 0.5f, 0.5f, 0.25f});
    REQUIRE(color[3] == 0.25f);
    REQUIRE(color[0] != 0.5f);
}

TEST_CASE("TextureContent: colour is decoded, data is not")
{
    REQUIRE(to_color_space(TextureContent::COLOR, RGBA_TEXTURE)
            == ColorSpace::SRGB);
    REQUIRE(to_color_space(TextureContent::COLOR, RGB_TEXTURE)
            == ColorSpace::SRGB);
    REQUIRE(to_color_space(TextureContent::DATA, RGBA_TEXTURE)
            == ColorSpace::LINEAR);
    REQUIRE(to_color_space(TextureContent::DATA, RGB_TEXTURE)
            == ColorSpace::LINEAR);
}

TEST_CASE("TextureContent: float colour is already linear")
{
    // Which is why callers name what a texture is for rather than naming a
    // colour space: COLOR is the right answer at either bit depth.
    constexpr TextureSourceFormat format = {TextureFormat::RGBA,
                                            TextureValueType::FLOAT};
    REQUIRE(to_color_space(TextureContent::COLOR, format)
            == ColorSpace::LINEAR);
}

TEST_CASE("TextureContent: single-channel colour has no sRGB format")
{
    // GLES 3.0 has no single-channel sRGB format, and silently storing the
    // texels linearly would be the quiet wrongness this API exists to stop.
    constexpr TextureSourceFormat format = {TextureFormat::R,
                                            TextureValueType::UINT8};
    REQUIRE_THROWS_AS(to_color_space(TextureContent::COLOR, format),
                      TungstenException);
    REQUIRE(to_color_space(TextureContent::DATA, format)
            == ColorSpace::LINEAR);
}
