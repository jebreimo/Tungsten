//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-01-25.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <vector>
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

    enum class StandardColorMaterial
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
        SLATE,
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

    ColorMaterial get_standard_color_material(StandardColorMaterial material);

    /**
     * Returns a std140 MaterialBlock of the builtin BlinnPhong family:
     * ambient, diffuse (w = opacity), specular (w = shininess).
     */
    std::vector<std::byte>
    make_blinn_phong_material_params(const ColorMaterial& material,
                                     float opacity = 1.0f);

    /**
     * Returns a std140 MaterialBlock of the builtin BlinnPhong family:
     * ambient, diffuse (w = opacity), specular (w = shininess).
     */
    std::vector<std::byte>
    make_blinn_phong_material_params(StandardColorMaterial material,
                                     float opacity = 1.0f);
} // Tungsten
