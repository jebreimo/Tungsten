//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-25.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    struct ColorMaterial
    {
        Xyz::Vector3F ambient = {0.5, 0.5, 0.5};
        Xyz::Vector3F diffuse = {0.5, 0.5, 0.5};
        Xyz::Vector3F specular = {0.5, 0.5, 0.5};
        float shininess = 32.0f;
    };

    struct TexMaterial
    {
        int32_t texture_unit = 0;
        Xyz::Vector3F specular = {0.5, 0.5, 0.5};
        float shininess = 32.0f;
    };

    struct SpecMaterial
    {
        Xyz::Vector3F ambient = {0.5, 0.5, 0.5};
        Xyz::Vector3F diffuse = {0.5, 0.5, 0.5};
        int32_t specular_unit = 0;
        float shininess = 32.0f;
    };

    struct TexSpecMaterial
    {
        int32_t texture_unit = 0;
        int32_t specular_unit = 1;
        float shininess = 32.0f;
    };

    enum class DefaultMaterial
    {
        EMERALD,
        JADE,
        OBSIDIAN,
        PEARL,
        RUBY,
        TURQUOISE,
        BRASS,
        BRONZE,
        CHROME,
        COPPER,
        GOLD,
        SILVER,
        BLACK_PLASTIC,
        CYAN_PLASTIC,
        GREEN_PLASTIC,
        RED_PLASTIC,
        WHITE_PLASTIC,
        YELLOW_PLASTIC,
        BLACK_RUBBER,
        CYAN_RUBBER,
        GREEN_RUBBER,
        RED_RUBBER,
        WHITE_RUBBER,
        YELLOW_RUBBER,
    };

    ColorMaterial get_default_material(DefaultMaterial material);
} // Tungsten
