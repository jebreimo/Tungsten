//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Animation/AnimationClip.hpp"

#include <cmath>
#include <limits>
#include <numbers>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;
using Catch::Approx;

namespace
{
    constexpr float PI = std::numbers::pi_v<float>;

    void require_near(const Xyz::Vector3F& actual,
                      const Xyz::Vector3F& expected)
    {
        for (unsigned i = 0; i < 3; ++i)
            REQUIRE(actual[i] == Approx(expected[i]).margin(1e-5));
    }

    // q and -q are the same rotation, so either sign passes.
    void require_same_rotation(const Xyz::QuaternionF& actual,
                               const Xyz::QuaternionF& expected)
    {
        const auto d = actual.w * expected.w + Xyz::dot(actual.v, expected.v);
        REQUIRE(std::abs(d) == Approx(1.0f).margin(1e-5));
    }

    Xyz::QuaternionF z_rotation(float angle)
    {
        return Xyz::make_quaternion<float>(angle, {0, 0, 1});
    }

    Vector3Track make_linear_track()
    {
        return {
            .interpolation = Interpolation::LINEAR,
            .times = {1, 2, 4},
            .values = {{0, 0, 0}, {10, 0, 0}, {10, 20, 0}}
        };
    }
}

TEST_CASE("AnimationClip: linear tracks interpolate between keyframes")
{
    const auto track = make_linear_track();

    require_near(sample(track, 1.5f), {5, 0, 0});
    require_near(sample(track, 3.0f), {10, 10, 0});
}

TEST_CASE("AnimationClip: keyframe times give the keyframe values")
{
    const auto track = make_linear_track();

    for (size_t i = 0; i < track.times.size(); ++i)
        require_near(sample(track, track.times[i]), track.values[i]);
}

TEST_CASE("AnimationClip: times outside the keyframes are clamped")
{
    const auto track = make_linear_track();

    require_near(sample(track, -5.0f), {0, 0, 0});
    require_near(sample(track, 100.0f), {10, 20, 0});
}

TEST_CASE("AnimationClip: NaN gives the first value")
{
    const auto track = make_linear_track();

    require_near(sample(track, std::numeric_limits<float>::quiet_NaN()),
                 {0, 0, 0});
}

TEST_CASE("AnimationClip: a single keyframe is constant")
{
    const Vector3Track track{.times = {2}, .values = {{1, 2, 3}}};

    require_near(sample(track, 0.0f), {1, 2, 3});
    require_near(sample(track, 2.0f), {1, 2, 3});
    require_near(sample(track, 9.0f), {1, 2, 3});
}

TEST_CASE("AnimationClip: step tracks hold each value until the next keyframe")
{
    auto track = make_linear_track();
    track.interpolation = Interpolation::STEP;

    require_near(sample(track, 1.99f), {0, 0, 0});
    require_near(sample(track, 2.0f), {10, 0, 0});
    require_near(sample(track, 3.99f), {10, 0, 0});
}

TEST_CASE("AnimationClip: cubic splines with zero tangents ease in and out")
{
    const Vector3Track track{
        .interpolation = Interpolation::CUBIC_SPLINE,
        .times = {0, 2},
        .values = {{0, 0, 0}, {8, 0, 0}},
        .in_tangents = {{0, 0, 0}, {0, 0, 0}},
        .out_tangents = {{0, 0, 0}, {0, 0, 0}}
    };

    // Smoothstep: 3u² - 2u³.
    require_near(sample(track, 1.0f), {4, 0, 0});
    constexpr float u = 0.25f;
    require_near(sample(track, 0.5f), {8 * (3 * u * u - 2 * u * u * u), 0, 0});
}

TEST_CASE("AnimationClip: cubic tangents are per second, not per segment")
{
    // A straight line at 3 units per second over a 2-second segment, so
    // the curve must reproduce it exactly — which it only does if the
    // tangents are scaled by the segment's length.
    const Vector3Track track{
        .interpolation = Interpolation::CUBIC_SPLINE,
        .times = {0, 2},
        .values = {{0, 0, 0}, {6, 0, 0}},
        .in_tangents = {{3, 0, 0}, {3, 0, 0}},
        .out_tangents = {{3, 0, 0}, {3, 0, 0}}
    };

    require_near(sample(track, 0.5f), {1.5f, 0, 0});
    require_near(sample(track, 1.0f), {3, 0, 0});
    require_near(sample(track, 1.5f), {4.5f, 0, 0});
}

TEST_CASE("AnimationClip: slerp moves at constant angular speed")
{
    const auto a = z_rotation(0);
    const auto b = z_rotation(PI / 2);

    require_same_rotation(slerp(a, b, 0.0f), a);
    require_same_rotation(slerp(a, b, 1.0f), b);
    require_same_rotation(slerp(a, b, 0.25f), z_rotation(PI / 8));
    require_same_rotation(slerp(a, b, 0.5f), z_rotation(PI / 4));
}

TEST_CASE("AnimationClip: slerp takes the shorter arc")
{
    // -b is the same rotation as b. Without the sign flip, the halfway point
    // would be on the 270° arc instead of the 90° one.
    const auto a = z_rotation(0);
    const auto b = z_rotation(PI / 2);

    require_same_rotation(slerp(a, -b, 0.5f), z_rotation(PI / 4));
}

TEST_CASE("AnimationClip: slerp between nearly equal rotations is unit length")
{
    const auto a = z_rotation(0);
    const auto b = z_rotation(1e-4f);

    const auto q = slerp(a, b, 0.5f);

    REQUIRE(Xyz::get_length(q) == Approx(1.0f).margin(1e-6));
    require_same_rotation(q, z_rotation(0.5e-4f));
}

TEST_CASE("AnimationClip: linear rotation tracks slerp")
{
    // Less than a half turn: at exactly 180° both arcs are equally short,
    // and rounding decides which one slerp takes.
    const RotationTrack track{
        .interpolation = Interpolation::LINEAR,
        .times = {0, 1},
        .values = {z_rotation(0), z_rotation(2 * PI / 3)}
    };

    require_same_rotation(sample(track, 0.5f), z_rotation(PI / 3));
}

TEST_CASE("AnimationClip: cubic rotation tracks are normalized")
{
    const RotationTrack track{
        .interpolation = Interpolation::CUBIC_SPLINE,
        .times = {0, 1},
        .values = {z_rotation(0), z_rotation(PI / 2)},
        .in_tangents = {{0, 0, 0, 0}, {0, 0, 0, 0}},
        .out_tangents = {{0, 0, 0, 0}, {0, 0, 0, 0}}
    };

    const auto q = sample(track, 0.5f);

    REQUIRE(Xyz::get_length(q) == Approx(1.0f).margin(1e-6));
    require_same_rotation(q, z_rotation(PI / 4));
}

TEST_CASE("AnimationClip: the duration is the last keyframe in any channel")
{
    const auto clip = make_animation_clip("walk", {
        {.target = 0, .path = AnimationPath::TRANSLATION,
         .track = make_linear_track()},
        {.target = 1, .path = AnimationPath::ROTATION,
         .track = RotationTrack{.times = {0, 6},
                                .values = {z_rotation(0), z_rotation(1)}}}
    });

    REQUIRE(clip.name == "walk");
    REQUIRE(clip.channels.size() == 2);
    REQUIRE(clip.duration == 6);
}

TEST_CASE("AnimationClip: invalid tracks are rejected")
{
    auto make = [](AnimationPath path,
                   std::variant<Vector3Track, RotationTrack> track)
    {
        return make_animation_clip("bad", {
            {.target = 0, .path = path, .track = std::move(track)}
        });
    };

    SECTION("empty")
    {
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, Vector3Track{}),
                          TungstenException);
    }

    SECTION("times not increasing")
    {
        auto track = make_linear_track();
        track.times = {1, 1, 4};
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, track), TungstenException);
    }

    SECTION("time not finite")
    {
        auto track = make_linear_track();
        track.times[2] = std::numeric_limits<float>::infinity();
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, track), TungstenException);
    }

    SECTION("value count differs from time count")
    {
        auto track = make_linear_track();
        track.values.pop_back();
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, track), TungstenException);
    }

    SECTION("cubic spline without tangents")
    {
        auto track = make_linear_track();
        track.interpolation = Interpolation::CUBIC_SPLINE;
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, track), TungstenException);
    }

    SECTION("linear with tangents")
    {
        auto track = make_linear_track();
        track.in_tangents = track.values;
        track.out_tangents = track.values;
        REQUIRE_THROWS_AS(make(AnimationPath::SCALE, track), TungstenException);
    }

    SECTION("rotation path with a vector track")
    {
        REQUIRE_THROWS_AS(make(AnimationPath::ROTATION, make_linear_track()),
                          TungstenException);
    }

    SECTION("translation path with a rotation track")
    {
        const RotationTrack track{.times = {0}, .values = {z_rotation(0)}};
        REQUIRE_THROWS_AS(make(AnimationPath::TRANSLATION, track),
                          TungstenException);
    }
}
