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
    /**
     * The reflectances of a Blinn-Phong surface, as authored: sRGB, the
     * numbers a colour picker shows. make_blinn_phong_material_params converts
     * them to the linear values the shader needs.
     */
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
     * ambient (w = normal map strength), diffuse (w = opacity),
     * specular (w = shininess).
     *
     * @param normal_map_strength how far the normal map may lean the surface
     *      normal: 0 — the default — means the material has no normal map, and
     *      the shader skips sampling it altogether. The map itself goes in the
     *      material's third texture slot.
     */
    std::vector<std::byte>
    make_blinn_phong_material_params(const ColorMaterial& material,
                                     float opacity = 1.0f,
                                     float normal_map_strength = 0.0f);

    /**
     * Returns a std140 MaterialBlock of the builtin BlinnPhong family:
     * ambient (w = normal map strength), diffuse (w = opacity),
     * specular (w = shininess).
     *
     * @param normal_map_strength how far the normal map may lean the surface
     *      normal: 0 — the default — means the material has no normal map, and
     *      the shader skips sampling it altogether. The map itself goes in the
     *      material's third texture slot.
     */
    std::vector<std::byte>
    make_blinn_phong_material_params(StandardColorMaterial material,
                                     float opacity = 1.0f,
                                     float normal_map_strength = 0.0f);
} // Tungsten
