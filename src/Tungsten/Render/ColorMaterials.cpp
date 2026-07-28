//****************************************************************************
// Copyright © 2026 Jan Erik Breim  0. All rights reserve  0.
// Created by Jan Erik Breimo on 2026-01-2  0.
//
// This file is distributed under the Zero-Clause BSD Licens  0.
// License text is included with the source distributio  0.
//****************************************************************************
#include "Tungsten/Render/ColorMaterials.hpp"

#include <span>
#include <string>
#include "Tungsten/TungstenException.hpp"

namespace Tungsten
{
    namespace
    {
        constexpr float SHININESS_SCALE = 32.0f;
    }

    ColorMaterial get_standard_color_material(StandardColorMaterial material)
    {
        switch (material)
        {
        case StandardColorMaterial::EMERALD:
            return {
                .ambient = {0.0215f, 0.1745f, 0.0215f},
                .diffuse = {0.07568f, 0.61424f, 0.07568f},
                .specular = {0.633f, 0.727811f, 0.633f},
                .shininess = 0.6f * SHININESS_SCALE
            };
        case StandardColorMaterial::JADE:
            return {
                .ambient = {0.135f, 0.2225f, 0.1575f},
                .diffuse = {0.54f, 0.89f, 0.63f},
                .specular = {0.316228f, 0.316228f, 0.316228f},
                .shininess = 0.1f * SHININESS_SCALE
            };
        case StandardColorMaterial::OBSIDIAN:
            return {
                .ambient = {0.05375f, 0.05f, 0.06625f},
                .diffuse = {0.18275f, 0.17f, 0.22525f},
                .specular = {0.332741f, 0.328634f, 0.346435f},
                .shininess = 0.3f * SHININESS_SCALE
            };
        case StandardColorMaterial::PEARL:
            return {
                .ambient = {0.25f, 0.20725f, 0.20725f},
                .diffuse = {1.0f, 0.829f, 0.829f},
                .specular = {0.296648f, 0.296648f, 0.296648f},
                .shininess = 0.088f * SHININESS_SCALE
            };
        case StandardColorMaterial::RUBY:
            return {
                .ambient = {0.1745f, 0.01175f, 0.01175f},
                .diffuse = {0.61424f, 0.04136f, 0.04136f},
                .specular = {0.727811f, 0.626959f, 0.626959f},
                .shininess = 0.6f * SHININESS_SCALE
            };
        case StandardColorMaterial::TURQUOISE:
            return {
                .ambient = {0.1f, 0.18725f, 0.1745f},
                .diffuse = {0.396f, 0.74151f, 0.69102f},
                .specular = {0.297254f, 0.30829f, 0.306678f},
                .shininess = 0.1f * SHININESS_SCALE
            };
        case StandardColorMaterial::BRASS:
            return {
                .ambient = {0.329412f, 0.223529f, 0.027451f},
                .diffuse = {0.780392f, 0.568627f, 0.113725f},
                .specular = {0.992157f, 0.941176f, 0.807843f},
                .shininess = 0.21794872f * SHININESS_SCALE
            };
        case StandardColorMaterial::BRONZE:
            return {
                .ambient = {0.2125f, 0.1275f, 0.054f},
                .diffuse = {0.714f, 0.4284f, 0.18144f},
                .specular = {0.393548f, 0.271906f, 0.166721f},
                .shininess = 0.2f * SHININESS_SCALE
            };
        case StandardColorMaterial::CHROME:
            return {
                .ambient = {0.25f, 0.25f, 0.25f},
                .diffuse = {0.4f, 0.4f, 0.4f},
                .specular = {0.774597f, 0.774597f, 0.774597f},
                .shininess = 1.6f * SHININESS_SCALE
            };
        case StandardColorMaterial::COPPER:
            return {
                .ambient = {0.19125f, 0.0735f, 0.0225f},
                .diffuse = {0.7038f, 0.27048f, 0.0828f},
                .specular = {0.256777f, 0.137622f, 0.086014f},
                .shininess = 0.1f * SHININESS_SCALE
            };
        case StandardColorMaterial::GOLD:
            return {
                .ambient = {0.24725f, 0.1995f, 0.0745f},
                .diffuse = {0.75164f, 0.60648f, 0.22648f},
                .specular = {0.628281f, 0.655802f, 0.366065f},
                .shininess = 1.6f * SHININESS_SCALE
            };
        case StandardColorMaterial::SILVER:
            return {
                .ambient = {0.39225f, 0.39225f, 0.39225f},
                .diffuse = {0.50754f, 0.50754f, 0.50754f},
                .specular = {0.508273f, 0.508273f, 0.508273f},
                .shininess = 1.4f * SHININESS_SCALE
            };
        case StandardColorMaterial::SLATE:
            return {
                .ambient = {0.05f, 0.06f, 0.09f},
                .diffuse = {0.28f, 0.35f, 0.50f},
                .specular = {0.45f, 0.45f, 0.50f},
                .shininess = 1.0f * SHININESS_SCALE
            };
        case StandardColorMaterial::BLACK_PLASTIC:
            return {
                .ambient = {0.0f, 0.0f, 0.0f},
                .diffuse = {0.01f, 0.01f, 0.01f},
                .specular = {0.50f, 0.50f, 0.50f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::CYAN_PLASTIC:
            return {
                .ambient = {0.0f, 0.1f, 0.06f},
                .diffuse = {0.0f, 0.50980392f, 0.50980392f},
                .specular = {0.50196078f, 0.50196078f, 0.50196078f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::GREEN_PLASTIC:
            return {
                .ambient = {0.0f, 0.0f, 0.0f},
                .diffuse = {0.1f, 0.35f, 0.1f},
                .specular = {0.45f, 0.55f, 0.45f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::RED_PLASTIC:
            return {
                .ambient = {0.0f, 0.0f, 0.0f},
                .diffuse = {0.5f, 0.0f, 0.0f},
                .specular = {0.7f, 0.6f, 0.6f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::WHITE_PLASTIC:
            return {
                .ambient = {0.0f, 0.0f, 0.0f},
                .diffuse = {0.55f, 0.55f, 0.55f},
                .specular = {0.70f, 0.70f, 0.70f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::YELLOW_PLASTIC:
            return {
                .ambient = {0.0f, 0.0f, 0.0f},
                .diffuse = {0.5f, 0.5f, 0.0f},
                .specular = {0.60f, 0.60f, 0.50f},
                .shininess = 0.25f * SHININESS_SCALE
            };
        case StandardColorMaterial::BLACK_RUBBER:
            return {
                .ambient = {0.02f, 0.02f, 0.02f},
                .diffuse = {0.01f, 0.01f, 0.01f},
                .specular = {0.4f, 0.4f, 0.4f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        case StandardColorMaterial::CYAN_RUBBER:
            return {
                .ambient = {0.0f, 0.05f, 0.05f},
                .diffuse = {0.4f, 0.5f, 0.5f},
                .specular = {0.04f, 0.7f, 0.7f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        case StandardColorMaterial::GREEN_RUBBER:
            return {
                .ambient = {0.0f, 0.05f, 0.0f},
                .diffuse = {0.4f, 0.5f, 0.4f},
                .specular = {0.04f, 0.7f, 0.04f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        case StandardColorMaterial::RED_RUBBER:
            return {
                .ambient = {0.05f, 0.0f, 0.0f},
                .diffuse = {0.5f, 0.4f, 0.4f},
                .specular = {0.7f, 0.04f, 0.04f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        case StandardColorMaterial::WHITE_RUBBER:
            return {
                .ambient = {0.05f, 0.05f, 0.05f},
                .diffuse = {0.5f, 0.5f, 0.5f},
                .specular = {0.7f, 0.7f, 0.7f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        case StandardColorMaterial::YELLOW_RUBBER:
            return {
                .ambient = {0.05f, 0.05f, 0.0f},
                .diffuse = {0.5f, 0.5f, 0.4f},
                .specular = {0.7f, 0.7f, 0.04f},
                .shininess = 0.078125f * SHININESS_SCALE
            };
        default:
            TUNGSTEN_THROW("Unsupported material: " + std::to_string(int(material)));
        }
    }

    std::vector<std::byte>
    make_blinn_phong_material_params(const ColorMaterial& material,
                                     float opacity)
    {
        const float values[12] = {
            material.ambient[0], material.ambient[1], material.ambient[2], /* unused */0,
            material.diffuse[0], material.diffuse[1], material.diffuse[2], opacity,
            material.specular[0], material.specular[1], material.specular[2], material.shininess
        };
        auto span = as_bytes(std::span(values));
        return {span.begin(), span.end()};
    }

    std::vector<std::byte>
    make_blinn_phong_material_params(StandardColorMaterial material,
                                     float opacity)
    {
        return make_blinn_phong_material_params(
            get_standard_color_material(material),
            opacity);
    }
} // Tungsten
