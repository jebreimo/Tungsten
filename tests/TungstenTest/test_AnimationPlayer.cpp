//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-10-21.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/Animation/AnimationPlayer.hpp"

#include <cmath>
#include <memory>
#include <numbers>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "Tungsten/TungstenException.hpp"
#include "Tungsten/SceneGraph/Scene.hpp"

using namespace Tungsten;
using Catch::Approx;

namespace
{
    constexpr float PI = std::numbers::pi_v<float>;

    // Moves target 0 from x = 0 to x = 10 over two seconds, and scales it
    // from 1 to 3 over the same time.
    std::shared_ptr<const AnimationClip> make_slide_clip()
    {
        return std::make_shared<const AnimationClip>(make_animation_clip(
            "slide", {
                {.target = 0, .path = AnimationPath::TRANSLATION,
                 .track = Vector3Track{.times = {0, 2},
                                       .values = {{0, 0, 0}, {10, 0, 0}}}},
                {.target = 0, .path = AnimationPath::SCALE,
                 .track = Vector3Track{.times = {0, 2},
                                       .values = {{1, 1, 1}, {3, 3, 3}}}}
            }));
    }

    // Turns target 1 half a turn about z over one second.
    std::shared_ptr<const AnimationClip> make_turn_clip()
    {
        return std::make_shared<const AnimationClip>(make_animation_clip(
            "turn", {
                {.target = 1, .path = AnimationPath::ROTATION,
                 .track = RotationTrack{
                     .times = {0, 1},
                     .values = {Xyz::QuaternionF(),
                                z_rotation(2 * PI / 3)}}}
            }));
    }

    struct PlayerFixture
    {
        PlayerFixture()
        {
            first = scene.create_node();
            second = scene.create_node();
            targets.nodes = {first, second};
            targets.rest_transforms = {
                Transform{.translation = {0, 5, 0},
                          .rotation = z_rotation(PI / 2)},
                Transform{.translation = {7, 8, 9}}
            };
        }

        Scene scene;
        NodeId first;
        NodeId second;
        AnimationTargets targets;
    };
}

TEST_CASE("AnimationPlayer: unanimated parts keep their rest values")
{
    PlayerFixture f;
    AnimationPlayer player(make_slide_clip(), f.targets);

    player.set_time(1);
    player.apply(f.scene);

    // Translation and scale are animated, and replaced as a whole. The
    // rotation is not, so it comes from the rest transform: a quarter turn
    // that takes x to y.
    const auto& m = f.scene.local_matrix(f.first);
    REQUIRE(m[0, 3] == Approx(5));
    REQUIRE(m[1, 3] == Approx(0));
    REQUIRE(m[0, 0] == Approx(0).margin(1e-6));
    REQUIRE(m[1, 0] == Approx(2));
}

TEST_CASE("AnimationPlayer: only the clip's nodes are written")
{
    PlayerFixture f;
    f.scene.set_local_transform(f.second, Transform{.translation = {1, 1, 1}});
    AnimationPlayer player(make_slide_clip(), f.targets);

    player.apply(f.scene);

    REQUIRE(f.scene.local_matrix(f.second)[0, 3] == Approx(1));
}

TEST_CASE("AnimationPlayer: looping wraps the time in both directions")
{
    AnimationPlayer player(make_slide_clip(), PlayerFixture().targets);

    player.advance(2.5f);
    REQUIRE(player.time() == Approx(0.5f));

    player.set_speed(-1);
    player.advance(1);
    REQUIRE(player.time() == Approx(1.5f));
    REQUIRE(!player.is_finished());
}

TEST_CASE("AnimationPlayer: playing once stops at the end")
{
    AnimationPlayer player(make_slide_clip(), PlayerFixture().targets,
                           PlaybackMode::ONCE);

    player.advance(1);
    REQUIRE(!player.is_finished());
    player.advance(5);
    REQUIRE(player.time() == 2);
    REQUIRE(player.is_finished());

    player.set_speed(-2);
    REQUIRE(!player.is_finished());
    player.advance(5);
    REQUIRE(player.time() == 0);
    REQUIRE(player.is_finished());
}

TEST_CASE("AnimationPlayer: speed scales the advance")
{
    AnimationPlayer player(make_slide_clip(), PlayerFixture().targets);
    player.set_speed(0.5f);

    player.advance(1);

    REQUIRE(player.time() == Approx(0.5f));
}

TEST_CASE("AnimationPlayer: update advances and applies")
{
    PlayerFixture f;
    AnimationPlayer player(make_turn_clip(), f.targets, PlaybackMode::ONCE);

    player.update(f.scene, 0.5f);

    // Halfway through a 120° turn: x goes to (cos 60°, sin 60°).
    const auto& m = f.scene.local_matrix(f.second);
    REQUIRE(m[0, 0] == Approx(0.5f));
    REQUIRE(m[1, 0] == Approx(std::sqrt(3.0f) / 2));
    REQUIRE(m[0, 3] == Approx(7));
}

TEST_CASE("AnimationPlayer: restore_rest_pose undoes the clip")
{
    PlayerFixture f;
    AnimationPlayer player(make_slide_clip(), f.targets);
    player.update(f.scene, 1);

    player.restore_rest_pose(f.scene);

    const auto& m = f.scene.local_matrix(f.first);
    REQUIRE(m[0, 3] == Approx(0));
    REQUIRE(m[1, 3] == Approx(5));
    REQUIRE(m[1, 0] == Approx(1));
}

TEST_CASE("AnimationPlayer: removed and missing nodes are skipped")
{
    PlayerFixture f;

    SECTION("removed")
    {
        AnimationPlayer player(make_slide_clip(), f.targets);
        f.scene.remove(f.first);
        REQUIRE_NOTHROW(player.update(f.scene, 1));
        REQUIRE_NOTHROW(player.restore_rest_pose(f.scene));
    }

    SECTION("null")
    {
        f.targets.nodes[0] = {};
        AnimationPlayer player(make_slide_clip(), f.targets);
        REQUIRE_NOTHROW(player.update(f.scene, 1));
    }
}

TEST_CASE("AnimationPlayer: invalid targets are rejected")
{
    PlayerFixture f;

    SECTION("null clip")
    {
        REQUIRE_THROWS_AS(AnimationPlayer(nullptr, f.targets),
                          TungstenException);
    }

    SECTION("target out of range")
    {
        f.targets.nodes.pop_back();
        f.targets.rest_transforms.pop_back();
        REQUIRE_THROWS_AS(AnimationPlayer(make_turn_clip(), f.targets),
                          TungstenException);
    }

    SECTION("mismatched sizes")
    {
        f.targets.rest_transforms.pop_back();
        REQUIRE_THROWS_AS(AnimationPlayer(make_slide_clip(), f.targets),
                          TungstenException);
    }
}

TEST_CASE("AnimationPlayer: a default player does nothing")
{
    Scene scene;
    AnimationPlayer player;

    REQUIRE(!player.clip());
    REQUIRE_NOTHROW(player.update(scene, 1));
    REQUIRE(player.time() == 0);
}
