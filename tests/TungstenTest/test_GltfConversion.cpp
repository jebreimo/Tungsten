//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/Import/GltfConversion.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/Color.hpp"
#include "Tungsten/SceneGraph/Transform.hpp"
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;
using namespace Tungsten::Detail;
using Catch::Approx;

TEST_CASE("GltfConversion: to_matrix transposes into row-major")
{
    // glTF stores the 16 floats column by column, so the translation is the
    // last four values; Xyz keeps it in the fourth column of a row-major
    // matrix.
    const std::array<float, 16> column_major{
        2, 0, 0, 0,
        0, 3, 0, 0,
        0, 0, 4, 0,
        5, 6, 7, 1
    };

    const auto m = to_matrix(column_major);

    REQUIRE(m[0, 0] == 2);
    REQUIRE(m[1, 1] == 3);
    REQUIRE(m[2, 2] == 4);
    REQUIRE(m[0, 3] == 5);
    REQUIRE(m[1, 3] == 6);
    REQUIRE(m[2, 3] == 7);
    REQUIRE(m[3, 3] == 1);
    REQUIRE(m[3, 0] == 0);
}

TEST_CASE("GltfConversion: to_quaternion reorders the components")
{
    // glTF: [x, y, z, w]. Xyz: (w, x, y, z).
    const auto q = to_quaternion({0.1f, 0.2f, 0.3f, 0.4f});

    REQUIRE(q.w == Approx(0.4f));
    REQUIRE(q.v[0] == Approx(0.1f));
    REQUIRE(q.v[1] == Approx(0.2f));
    REQUIRE(q.v[2] == Approx(0.3f));
}

TEST_CASE("GltfConversion: the identity rotation survives the trip")
{
    const auto q = to_quaternion({0, 0, 0, 1});
    const auto m = Transform{.rotation = q}.make_matrix();

    for (unsigned row = 0; row < 4; ++row)
    {
        for (unsigned col = 0; col < 4; ++col)
            REQUIRE(m[row, col] == Approx(row == col ? 1.0f : 0.0f));
    }
}

TEST_CASE("GltfConversion: a base color survives the sRGB round trip")
{
    // to_color_material encodes to sRGB because
    // make_blinn_phong_material_params decodes again. A dielectric keeps its
    // base color, so decoding the result must give back what glTF held.
    const Xyz::Vector4F base_color{0.25f, 0.5f, 0.75f, 1.0f};

    const auto material = to_color_material(base_color, 0.0f, 0.5f);
    const auto diffuse = srgb_to_linear(material.diffuse);

    REQUIRE(diffuse[0] == Approx(0.25f).margin(1e-4));
    REQUIRE(diffuse[1] == Approx(0.5f).margin(1e-4));
    REQUIRE(diffuse[2] == Approx(0.75f).margin(1e-4));
}

TEST_CASE("GltfConversion: metal keeps its color in the specular term")
{
    const Xyz::Vector4F base_color{0.9f, 0.6f, 0.3f, 1.0f};

    const auto dielectric = to_color_material(base_color, 0.0f, 0.5f);
    const auto metal = to_color_material(base_color, 1.0f, 0.5f);

    // A dielectric reflects a colorless 4%; a metal reflects its own color.
    const auto dielectric_specular = srgb_to_linear(dielectric.specular);
    REQUIRE(dielectric_specular[0] == Approx(0.04f).margin(1e-4));
    REQUIRE(dielectric_specular[1] == Approx(0.04f).margin(1e-4));

    const auto metal_specular = srgb_to_linear(metal.specular);
    REQUIRE(metal_specular[0] == Approx(0.9f).margin(1e-4));
    REQUIRE(metal_specular[1] == Approx(0.6f).margin(1e-4));

    // ... and gives up most of its diffuse reflectance for it.
    REQUIRE(srgb_to_linear(metal.diffuse)[0]
            < srgb_to_linear(dielectric.diffuse)[0]);
}

TEST_CASE("GltfConversion: shininess falls as roughness rises")
{
    REQUIRE(roughness_to_shininess(0.0f) == 256.0f);
    REQUIRE(roughness_to_shininess(1.0f) == 1.0f);
    REQUIRE(roughness_to_shininess(0.3f) > roughness_to_shininess(0.6f));
    // Never outside the range the shader can represent, whatever it is given.
    REQUIRE(roughness_to_shininess(-1.0f) <= 256.0f);
    REQUIRE(roughness_to_shininess(10.0f) >= 1.0f);
}

TEST_CASE("GltfConversion: complete_primitive flat-shades a normal-less mesh")
{
    // Two triangles of a square in the z = 0 plane, wound counterclockwise.
    PrimitiveData data;
    data.positions = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    data.indices = {0, 1, 2, 0, 2, 3};

    complete_primitive(data);

    // The triangles are separated so each corner can carry its face normal.
    REQUIRE(data.positions.size() == 6);
    REQUIRE(data.normals.size() == 6);
    REQUIRE(data.tex_coords.size() == 6);
    REQUIRE(data.indices == std::vector<uint32_t>{0, 1, 2, 3, 4, 5});

    for (const auto& normal : data.normals)
    {
        REQUIRE(normal[0] == Approx(0.0f));
        REQUIRE(normal[1] == Approx(0.0f));
        REQUIRE(normal[2] == Approx(1.0f));
    }
}

TEST_CASE("GltfConversion: complete_primitive keeps supplied normals")
{
    PrimitiveData data;
    data.positions = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}};
    data.normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    data.indices = {0, 1, 2};

    complete_primitive(data);

    REQUIRE(data.positions.size() == 3);
    REQUIRE(data.normals[0][1] == Approx(1.0f));
    // Only the missing texture coordinates are filled in.
    REQUIRE(data.tex_coords.size() == 3);
    REQUIRE(data.tex_coords[0][0] == Approx(0.0f));
}

TEST_CASE("GltfConversion: a degenerate triangle still yields a unit normal")
{
    PrimitiveData data;
    data.positions = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}};
    data.indices = {0, 1, 2};

    complete_primitive(data);

    REQUIRE(Xyz::get_length(data.normals[0]) == Approx(1.0f));
}

TEST_CASE("GltfConversion: interleave packs position, normal and texcoord")
{
    PrimitiveData data;
    data.positions = {{1, 2, 3}, {4, 5, 6}};
    data.normals = {{0, 1, 0}, {1, 0, 0}};
    data.tex_coords = {{0.25f, 0.5f}, {0.75f, 1.0f}};

    const auto vertices = interleave(data);

    REQUIRE(vertices.size() == 2 * GLTF_VERTEX_STRIDE);
    const std::vector<float> expected{
        1, 2, 3, 0, 1, 0, 0.25f, 0.5f,
        4, 5, 6, 1, 0, 0, 0.75f, 1.0f
    };
    REQUIRE(vertices == expected);
}

TEST_CASE("GltfConversion: make_line_indices lists a shared edge once")
{
    // Two triangles of a square, sharing the diagonal 0-2.
    const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    const auto lines = make_line_indices(indices, TopologyType::TRIANGLES);

    REQUIRE(lines == std::vector<uint32_t>{0, 1, 0, 2, 0, 3, 1, 2, 2, 3});
}

TEST_CASE("GltfConversion: make_line_indices unrolls strips and fans")
{
    const std::vector<uint32_t> indices = {0, 1, 2, 3};

    // A strip's triangles are 0-1-2 and 1-2-3; a fan's are 0-1-2 and 0-2-3.
    REQUIRE(make_line_indices(indices, TopologyType::TRIANGLE_STRIP)
            == std::vector<uint32_t>{0, 1, 0, 2, 1, 2, 1, 3, 2, 3});
    REQUIRE(make_line_indices(indices, TopologyType::TRIANGLE_FAN)
            == std::vector<uint32_t>{0, 1, 0, 2, 0, 3, 1, 2, 2, 3});
}

TEST_CASE("GltfConversion: make_specular_mask folds both channels into grey")
{
    // glTF packs roughness in green and metalness in blue. The red channel is
    // occlusion or unused, and must not reach the result.
    const std::vector<uint8_t> pixels{
        255, 0, 255,    // polished metal: fully reflective
        255, 255, 255,  // rough metal: not reflective at all
        255, 0, 0,      // polished dielectric: no metal, so no mask
        0, 128, 255     // half-rough metal
    };

    const auto mask = make_specular_mask(pixels.data(), 4, 3);

    REQUIRE(mask.size() == 4 * 3);
    // Grey: the shader reads .rgb, so all three channels must agree.
    for (size_t i = 0; i < 4; ++i)
    {
        REQUIRE(mask[i * 3] == mask[i * 3 + 1]);
        REQUIRE(mask[i * 3] == mask[i * 3 + 2]);
    }
    REQUIRE(mask[0] == 255);
    REQUIRE(mask[3] == 0);
    REQUIRE(mask[6] == 0);
    REQUIRE(mask[9] == 127);
}

TEST_CASE("GltfConversion: make_specular_mask copes with fewer channels")
{
    const std::vector<uint8_t> grey{64, 200};

    const auto mask = make_specular_mask(grey.data(), 2, 1);

    // With no green or blue to read, the missing channels count as full, which
    // leaves a mask of zero rather than reading past the end of the pixel.
    REQUIRE(mask.size() == 2 * 3);
    REQUIRE(mask[0] == 0);
    REQUIRE(mask[3] == 0);
}

TEST_CASE("GltfConversion: linear outputs become the track's values")
{
    const auto track = make_vector3_track(Interpolation::LINEAR, {0, 1},
                                          {{1, 2, 3}, {4, 5, 6}});

    REQUIRE(track.times == std::vector<float>{0, 1});
    REQUIRE(track.values[1] == Xyz::Vector3F(4, 5, 6));
    REQUIRE(track.in_tangents.empty());
    REQUIRE(track.out_tangents.empty());
}

TEST_CASE("GltfConversion: cubic outputs are split into tangents and values")
{
    // glTF order per keyframe: in-tangent, value, out-tangent.
    const auto track = make_vector3_track(
        Interpolation::CUBIC_SPLINE, {0, 1},
        {{1, 0, 0}, {2, 0, 0}, {3, 0, 0}, {4, 0, 0}, {5, 0, 0}, {6, 0, 0}});

    REQUIRE(track.in_tangents[0][0] == 1);
    REQUIRE(track.values[0][0] == 2);
    REQUIRE(track.out_tangents[0][0] == 3);
    REQUIRE(track.in_tangents[1][0] == 4);
    REQUIRE(track.values[1][0] == 5);
    REQUIRE(track.out_tangents[1][0] == 6);
}

TEST_CASE("GltfConversion: the output count must match the interpolation")
{
    REQUIRE_THROWS_AS(make_vector3_track(Interpolation::LINEAR, {0, 1},
                                         {{0, 0, 0}}),
                      TungstenException);
    REQUIRE_THROWS_AS(make_vector3_track(Interpolation::CUBIC_SPLINE, {0, 1},
                                         {{0, 0, 0}, {0, 0, 0}}),
                      TungstenException);
}

TEST_CASE("GltfConversion: rotation outputs are reordered and normalized")
{
    // [x, y, z, w], twice the length of a unit quaternion.
    const auto track = make_rotation_track(Interpolation::STEP, {0},
                                           {{0, 0, 1.2f, 1.6f}});

    const auto& q = track.values[0];
    REQUIRE(q.w == Approx(0.8f));
    REQUIRE(q.v[2] == Approx(0.6f));
}

TEST_CASE("GltfConversion: rotation tangents are not normalized")
{
    const auto track = make_rotation_track(
        Interpolation::CUBIC_SPLINE, {0},
        {{0, 0, 0, 2}, {0, 0, 0, 1}, {0, 0, 0, 3}});

    REQUIRE(track.in_tangents[0].w == 2);
    REQUIRE(track.out_tangents[0].w == 3);
}

namespace
{
    PrimitiveData make_skinned_triangle()
    {
        PrimitiveData data;
        data.positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
        data.normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};
        data.tex_coords = {{0, 0}, {1, 0}, {0, 1}};
        data.joints = {{0, 0, 0, 0}, {1, 0, 0, 0}, {2, 3, 0, 0}};
        data.weights = {{1, 0, 0, 0}, {1, 0, 0, 0}, {0.5f, 0.5f, 0, 0}};
        data.indices = {0, 1, 2};
        return data;
    }
}

TEST_CASE("GltfConversion: skinned vertices carry joints and weights")
{
    const auto data = make_skinned_triangle();

    const auto vertices = interleave(data);

    REQUIRE(vertices.size() == 3 * GLTF_SKINNED_VERTEX_STRIDE);
    const auto* last = &vertices[2 * GLTF_SKINNED_VERTEX_STRIDE];
    REQUIRE(last[1] == 1);      // position y
    REQUIRE(last[8] == 2);      // first joint
    REQUIRE(last[9] == 3);      // second joint
    REQUIRE(last[12] == 0.5f);  // first weight
}

TEST_CASE("GltfConversion: rigid vertices keep the short format")
{
    auto data = make_skinned_triangle();
    data.joints.clear();
    data.weights.clear();

    REQUIRE(interleave(data).size() == 3 * GLTF_VERTEX_STRIDE);
}

TEST_CASE("GltfConversion: flat shading keeps each vertex's joints")
{
    auto data = make_skinned_triangle();
    data.normals.clear();
    // The same corner twice, so the unrolled triangle has to copy its joints.
    data.indices = {0, 2, 2};

    complete_primitive(data);

    REQUIRE(data.joints.size() == 3);
    REQUIRE(data.weights.size() == 3);
    REQUIRE(data.joints[1] == Xyz::Vector4F(2, 3, 0, 0));
    REQUIRE(data.joints[2] == Xyz::Vector4F(2, 3, 0, 0));
}

TEST_CASE("GltfConversion: weights are scaled to sum to one")
{
    std::vector<Xyz::Vector4F> weights{{1, 1, 2, 0}, {0, 0, 0, 0}};

    normalize_weights(weights);

    REQUIRE(weights[0] == Xyz::Vector4F(0.25f, 0.25f, 0.5f, 0));
    // No weight at all goes to the first joint.
    REQUIRE(weights[1] == Xyz::Vector4F(1, 0, 0, 0));
}

TEST_CASE("GltfConversion: joint indices must fit the skin")
{
    REQUIRE(are_valid_joint_indices({{0, 1, 2, 3}}, 4));
    REQUIRE(!are_valid_joint_indices({{0, 1, 2, 4}}, 4));
    REQUIRE(!are_valid_joint_indices({{0, 1, 2.5f, 3}}, 4));
    REQUIRE(!are_valid_joint_indices({{-1, 0, 0, 0}}, 4));
}
