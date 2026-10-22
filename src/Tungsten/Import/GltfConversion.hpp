//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-09-22.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <vector>
#include <Xyz/Matrix.hpp>
#include <Xyz/Quaternion.hpp>
#include <Xyz/Vector.hpp>
#include "Tungsten/Animation/AnimationClip.hpp"
#include "Tungsten/Gpu/GpuTypes.hpp"
#include "Tungsten/Resources/ColorMaterials.hpp"

/**
 * The parts of the glTF importer that are pure data conversion. They name no
 * fastgltf type, which is what lets them be tested without a glTF file or a GL
 * context — and they are where the importer's easy-to-miss mistakes live:
 * glTF and Xyz disagree about matrix order, about quaternion component order,
 * and about whether a colour is linear or sRGB.
 */
namespace Tungsten::Detail
{
    /** Floats per vertex in the interleaved format the importer emits for a
     *  rigid mesh: position (3), normal (3), texture coordinate (2). */
    constexpr size_t GLTF_VERTEX_STRIDE = 8;

    /** Floats per vertex in the format for a skinned mesh: the rigid format
     *  followed by four joint indices and four weights. */
    constexpr size_t GLTF_SKINNED_VERTEX_STRIDE = 16;

    /**
     * Converts a glTF node matrix to an Xyz matrix.
     *
     * glTF stores the 16 floats in column-major order; Xyz::Matrix4F is
     * row-major with the translation in the fourth column, so this transposes.
     */
    Xyz::Matrix4F to_matrix(const std::array<float, 16>& column_major);

    /**
     * Converts a glTF node rotation to an Xyz quaternion.
     *
     * glTF stores a rotation as [x, y, z, w]; Xyz::QuaternionF is (w, x, y, z).
     */
    Xyz::QuaternionF to_quaternion(const std::array<float, 4>& xyzw);

    /**
     * Approximates a glTF metallic-roughness material as a Blinn-Phong one.
     *
     * @param base_color the base color factor, linear as glTF stores it. The
     *      returned ColorMaterial is sRGB, because that is what
     *      make_blinn_phong_material_params expects.
     */
    ColorMaterial to_color_material(const Xyz::Vector4F& base_color,
                                    float metallic, float roughness);

    /**
     * The Blinn-Phong specular exponent that corresponds to a roughness,
     * clamped to a range the shader can represent.
     */
    float roughness_to_shininess(float roughness);

    /**
     * The attributes of one glTF primitive, already widened to float.
     */
    struct PrimitiveData
    {
        std::vector<Xyz::Vector3F> positions;
        std::vector<Xyz::Vector3F> normals;
        std::vector<Xyz::Vector2F> tex_coords;
        /** Empty for a rigid mesh. Joint indices, widened to float. */
        std::vector<Xyz::Vector4F> joints;
        /** Empty for a rigid mesh; otherwise as long as joints. */
        std::vector<Xyz::Vector4F> weights;
        std::vector<uint32_t> indices;
    };

    /**
     * Fills in the attributes the Blinn-Phong family requires but the file did
     * not provide.
     *
     * Missing normals make the primitive flat-shaded, which is what the glTF
     * specification prescribes: the triangles are separated so that each gets
     * its own geometric normal, which grows the vertex count. Missing texture
     * coordinates become zeroes. Joints and weights, if any, follow the
     * vertices they belong to.
     */
    void complete_primitive(PrimitiveData& data);

    /**
     * Interleaves a primitive's attributes into the importer's vertex format:
     * the skinned one if it has joints, otherwise the rigid one.
     *
     * Expects complete_primitive to have run, i.e. all the attribute arrays
     * to be the same length.
     */
    std::vector<float> interleave(const PrimitiveData& data);

    /**
     * The line segments along the edges of the triangles that @a indices
     * assemble into under @a topology, which must be one of the triangle
     * topologies. An edge shared by several triangles is listed once.
     */
    std::vector<uint32_t> make_line_indices(std::span<const uint32_t> indices,
                                            TopologyType topology);

    /**
     * Folds a glTF metallic-roughness image into the single greyscale
     * reflectance mask that Blinn-Phong's specular map can express:
     * metallic * (1 - roughness), replicated across all three channels.
     *
     * The glTF image cannot be used as it is. Its green channel is roughness
     * and its blue metalness, so handing it to u_specular_map tints every
     * highlight green-blue — and dropping it instead leaves rough metal with
     * the same mirror highlight as polished metal, which washes the surface
     * out. Folding both channels into one number keeps the distinction the
     * shader can actually draw.
     *
     * Three channels rather than one because GLES samples a single-channel
     * texture as (r, 0, 0): the shader reads .rgb, so a red mask would tint
     * the highlight as badly as the original.
     *
     * @param channels bytes per pixel in @a pixels — bytes, not Yimage's
     *      bits. Anything past the third is ignored.
     */
    std::vector<uint8_t> make_specular_mask(const uint8_t* pixels,
                                            size_t pixel_count, size_t channels);

    /**
     * Scales each vertex's weights to sum to one. glTF requires that they
     * do, but weights stored as 8- or 16-bit integers rarely sum to exactly
     * one after widening. A vertex with no weight at all is given entirely
     * to its first joint, rather than collapsing to the origin.
     */
    void normalize_weights(std::vector<Xyz::Vector4F>& weights);

    /**
     * Returns true if every one of @a joints is a whole number that indexes
     * a skin of @a joint_count joints.
     */
    bool are_valid_joint_indices(const std::vector<Xyz::Vector4F>& joints,
                                 size_t joint_count);

    /**
     * Builds a translation or scale track from a glTF animation sampler's
     * input and output.
     *
     * For CUBIC_SPLINE, glTF stores three outputs per keyframe — in-tangent,
     * value, out-tangent — which this splits into the track's three arrays.
     *
     * @throw TungstenException if the output count does not match the input
     *      count and the interpolation.
     */
    Vector3Track make_vector3_track(Interpolation interpolation,
                                    std::vector<float> times,
                                    const std::vector<Xyz::Vector3F>& outputs);

    /**
     * Builds a rotation track from a glTF animation sampler's input and
     * output, as make_vector3_track does.
     *
     * The outputs are quaternions in glTF's [x, y, z, w] order. The values
     * are normalized: they may have been stored as 8- or 16-bit integers,
     * and widening those rarely gives a unit quaternion back.
     */
    RotationTrack make_rotation_track(Interpolation interpolation,
                                      std::vector<float> times,
                                      const std::vector<Xyz::Vector4F>& outputs);
} // Tungsten::Detail
