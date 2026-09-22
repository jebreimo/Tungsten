//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "GltfConversion.hpp"

#include <algorithm>
#include <cmath>
#include "Tungsten/Color.hpp"

namespace Tungsten::Detail
{
    namespace
    {
        // How much of the base color a fully metallic surface gives up as
        // diffuse reflectance. Physically it should give up all of it, but
        // Blinn-Phong has neither image-based lighting nor a Fresnel term to
        // put the energy back: a metal that follows the rule exactly has
        // nothing left but a white highlight, and loses the colour that makes
        // it recognisable.
        constexpr float METALLIC_DIFFUSE_LOSS = 0.4f;

        // The dielectric normal-incidence reflectance glTF assumes, and the
        // value the specular color interpolates from.
        constexpr float DIELECTRIC_SPECULAR = 0.04f;

        // Roughness zero would send the exponent to infinity.
        constexpr float MIN_ROUGHNESS = 0.03f;

        constexpr float MIN_SHININESS = 1.0f;
        constexpr float MAX_SHININESS = 256.0f;
    }

    Xyz::Matrix4F to_matrix(const std::array<float, 16>& m)
    {
        return {
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15]
        };
    }

    Xyz::QuaternionF to_quaternion(const std::array<float, 4>& q)
    {
        return {q[3], q[0], q[1], q[2]};
    }

    float roughness_to_shininess(float roughness)
    {
        // The usual microfacet-to-Blinn-Phong bridge: the GGX width is
        // roughness squared, and the exponent that peaks equally sharply is
        // 2/width^2 - 2.
        const auto alpha = std::max(roughness, MIN_ROUGHNESS);
        const auto shininess = 2.0f / std::pow(alpha, 4.0f) - 2.0f;
        return std::clamp(shininess, MIN_SHININESS, MAX_SHININESS);
    }

    ColorMaterial to_color_material(const Xyz::Vector4F& base_color,
                                    float metallic, float roughness)
    {
        const Xyz::Vector3F color{base_color[0], base_color[1], base_color[2]};
        const auto m = std::clamp(metallic, 0.0f, 1.0f);

        const auto diffuse = color * (1 - METALLIC_DIFFUSE_LOSS * m);
        // Metals reflect their own color, dielectrics a colorless 4%.
        const auto specular = Xyz::Vector3F(DIELECTRIC_SPECULAR) * (1 - m)
                              + color * m;

        // ColorMaterial is authored in sRGB and converted to linear again by
        // make_blinn_phong_material_params; glTF's factors are already linear,
        // so they have to be encoded on the way in or the round trip brightens
        // every material.
        return {
            .ambient = linear_to_srgb(diffuse * 0.15f),
            .diffuse = linear_to_srgb(diffuse),
            .specular = linear_to_srgb(specular),
            .shininess = roughness_to_shininess(roughness)
        };
    }

    void complete_primitive(PrimitiveData& data)
    {
        if (data.normals.empty())
        {
            // Flat shading: one vertex per triangle corner, so that the three
            // corners can share a face normal without affecting a neighbour.
            const auto old_positions = std::move(data.positions);
            const auto old_tex_coords = std::move(data.tex_coords);
            const auto old_indices = std::move(data.indices);
            const auto has_tex_coords = !old_tex_coords.empty();

            data.positions.clear();
            data.tex_coords.clear();
            data.indices.clear();
            data.positions.reserve(old_indices.size());
            data.normals.reserve(old_indices.size());
            data.indices.reserve(old_indices.size());

            for (size_t i = 0; i + 2 < old_indices.size(); i += 3)
            {
                const auto& a = old_positions[old_indices[i]];
                const auto& b = old_positions[old_indices[i + 1]];
                const auto& c = old_positions[old_indices[i + 2]];

                auto normal = Xyz::cross(b - a, c - a);
                // A degenerate triangle has no normal to speak of; any unit
                // vector keeps the vertex data well-formed.
                normal = Xyz::get_length_squared(normal) > 0
                             ? Xyz::normalize(normal)
                             : Xyz::Vector3F{0, 1, 0};

                for (size_t j = 0; j < 3; ++j)
                {
                    const auto index = old_indices[i + j];
                    data.positions.push_back(old_positions[index]);
                    data.normals.push_back(normal);
                    if (has_tex_coords)
                        data.tex_coords.push_back(old_tex_coords[index]);
                    data.indices.push_back(uint32_t(data.indices.size()));
                }
            }
        }

        if (data.tex_coords.size() != data.positions.size())
            data.tex_coords.resize(data.positions.size(), {0, 0});
    }

    std::vector<float> interleave(const PrimitiveData& data)
    {
        std::vector<float> result(data.positions.size() * GLTF_VERTEX_STRIDE);
        for (size_t i = 0; i < data.positions.size(); ++i)
        {
            auto* v = &result[i * GLTF_VERTEX_STRIDE];
            v[0] = data.positions[i][0];
            v[1] = data.positions[i][1];
            v[2] = data.positions[i][2];
            v[3] = data.normals[i][0];
            v[4] = data.normals[i][1];
            v[5] = data.normals[i][2];
            v[6] = data.tex_coords[i][0];
            v[7] = data.tex_coords[i][1];
        }
        return result;
    }

    std::vector<uint8_t> make_specular_mask(const uint8_t* pixels,
                                            size_t pixel_count, size_t channels)
    {
        std::vector<uint8_t> mask(pixel_count * 3);
        for (size_t i = 0; i < pixel_count; ++i)
        {
            const auto* texel = pixels + i * channels;
            // glTF packs roughness in green and metalness in blue.
            const auto roughness = channels > 1 ? texel[1] : uint8_t(255);
            const auto metallic = channels > 2 ? texel[2] : uint8_t(255);
            const auto value = uint8_t(metallic * (255 - roughness) / 255);
            mask[i * 3] = value;
            mask[i * 3 + 1] = value;
            mask[i * 3 + 2] = value;
        }
        return mask;
    }

    Xyz::BBox3F make_bounds(const std::vector<Xyz::Vector3F>& positions)
    {
        Xyz::BBox3F bounds;
        for (const auto& position : positions)
            bounds += position;
        return bounds;
    }
} // Tungsten::Detail
