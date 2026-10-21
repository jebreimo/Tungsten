//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <Xyz/Quaternion.hpp>
#include <Xyz/Vector.hpp>

namespace Tungsten
{
    /**
     * How a track's value changes between two keyframes. The three modes
     * glTF defines.
     */
    enum class Interpolation : uint8_t
    {
        /** Holds each keyframe's value until the next keyframe. */
        STEP,
        /** Interpolates linearly; rotations are slerped. */
        LINEAR,
        /** A cubic Hermite spline through the values, with explicit
         *  tangents. */
        CUBIC_SPLINE
    };

    /**
     * One animated property's keyframes: the value it has at each of a
     * strictly increasing sequence of times, in seconds.
     *
     * in_tangents and out_tangents are only used by CUBIC_SPLINE, which needs
     * one of each per keyframe, and must be empty otherwise. They are the
     * tangents as glTF stores them, i.e. per second rather than per segment.
     */
    template <typename T>
    struct AnimationTrack
    {
        Interpolation interpolation = Interpolation::LINEAR;
        std::vector<float> times;
        std::vector<T> values;
        std::vector<T> in_tangents;
        std::vector<T> out_tangents;
    };

    using Vector3Track = AnimationTrack<Xyz::Vector3F>;
    using RotationTrack = AnimationTrack<Xyz::QuaternionF>;

    /**
     * Returns the track's value at @a time. Times before the first keyframe
     * give its value, times after the last give the last value.
     *
     * The track must be valid, as make_animation_clip ensures: non-empty,
     * with strictly increasing times and matching array sizes.
     */
    [[nodiscard]]
    Xyz::Vector3F sample(const Vector3Track& track, float time);

    /**
     * Returns the track's rotation at @a time, always a unit quaternion if
     * the keyframes are. Otherwise as the Vector3Track overload.
     */
    [[nodiscard]]
    Xyz::QuaternionF sample(const RotationTrack& track, float time);

    /**
     * Spherical linear interpolation from @a a to @a b, along the shorter of
     * the two arcs between them.
     */
    [[nodiscard]]
    Xyz::QuaternionF slerp(const Xyz::QuaternionF& a,
                           const Xyz::QuaternionF& b,
                           float t);

    /**
     * The part of a node's local transform a channel animates.
     */
    enum class AnimationPath : uint8_t
    {
        TRANSLATION,
        ROTATION,
        SCALE
    };

    /**
     * One track bound to one property of one node.
     *
     * The target is an index rather than a NodeId, so that one clip can drive
     * every instance of the hierarchy it was made for: each instance maps the
     * indices to its own nodes.
     */
    struct AnimationChannel
    {
        uint32_t target = 0;
        AnimationPath path = AnimationPath::TRANSLATION;
        /** A RotationTrack for ROTATION, a Vector3Track otherwise. */
        std::variant<Vector3Track, RotationTrack> track;
    };

    /**
     * A named set of channels that play together. Create it with
     * make_animation_clip, which validates the tracks and computes the
     * duration.
     */
    struct AnimationClip
    {
        std::string name;
        std::vector<AnimationChannel> channels;
        /** The time of the last keyframe in any of the channels. */
        float duration = 0;
    };

    /**
     * Creates a clip from @a channels.
     *
     * @throw TungstenException if a track is empty, its times are not
     *      strictly increasing or not finite, its array sizes do not match
     *      its interpolation, or its type does not match its channel's path.
     */
    [[nodiscard]]
    AnimationClip make_animation_clip(std::string name,
                                      std::vector<AnimationChannel> channels);
} // Tungsten
