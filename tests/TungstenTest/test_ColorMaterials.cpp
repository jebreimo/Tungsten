//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-06.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Resources/ColorMaterials.hpp"

#include "Tungsten/Color.hpp"

#include <cstring>
#include <vector>
#include <catch2/catch_test_macros.hpp>

using namespace Tungsten;

namespace
{
    // The blob is uploaded to the MaterialBlock UBO verbatim, so these indexes
    // are the contract with BlinnPhong-frag.glsl. Nothing in the type system
    // holds the two in step — hence this test.
    std::vector<float> floats(const std::vector<std::byte>& blob)
    {
        std::vector<float> values(blob.size() / sizeof(float));
        std::memcpy(values.data(), blob.data(), blob.size());
        return values;
    }

    const ColorMaterial MATERIAL = {
        .ambient = {0.1f, 0.2f, 0.3f},
        .diffuse = {0.4f, 0.5f, 0.6f},
        .specular = {0.7f, 0.8f, 0.9f},
        .shininess = 16.0f
    };
}

TEST_CASE("BlinnPhong params: three vec4s in std140 order")
{
    const auto values = floats(make_blinn_phong_material_params(MATERIAL));

    // A ColorMaterial is authored in sRGB and the MaterialBlock is read by a
    // shader that works in linear space, so the values cross over here.
    REQUIRE(values.size() == 12);
    REQUIRE(values[0] == srgb_to_linear(0.1f));
    REQUIRE(values[1] == srgb_to_linear(0.2f));
    REQUIRE(values[2] == srgb_to_linear(0.3f));
    REQUIRE(values[4] == srgb_to_linear(0.4f));
    REQUIRE(values[6] == srgb_to_linear(0.6f));
    REQUIRE(values[8] == srgb_to_linear(0.7f));
    REQUIRE(values[10] == srgb_to_linear(0.9f));
}

TEST_CASE("BlinnPhong params: the w components carry the scalars")
{
    const auto values = floats(
        make_blinn_phong_material_params(MATERIAL, 0.25f, 0.75f));

    REQUIRE(values[3] == 0.75f);   // u_ambient.w  — normal map strength
    REQUIRE(values[7] == 0.25f);   // u_diffuse.w  — opacity
    REQUIRE(values[11] == 16.0f);  // u_specular.w — shininess
}

TEST_CASE("BlinnPhong params: a material has no normal map by default")
{
    // Zero is what makes the shader skip the sampler and the derivatives, so
    // every material written before normal maps existed keeps its old look.
    const auto values = floats(make_blinn_phong_material_params(MATERIAL));

    REQUIRE(values[3] == 0.0f);
    REQUIRE(values[7] == 1.0f);
}

TEST_CASE("BlinnPhong params: the standard-material overload forwards both")
{
    const auto values = floats(make_blinn_phong_material_params(
        StandardColorMaterial::GOLD, 0.5f, 2.0f));

    REQUIRE(values[3] == 2.0f);
    REQUIRE(values[7] == 0.5f);
    REQUIRE(values[11] == get_standard_color_material(
                              StandardColorMaterial::GOLD).shininess);
}
