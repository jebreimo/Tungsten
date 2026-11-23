//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-11-23.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "Tungsten/FixedStepClock.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "Tungsten/TungstenException.hpp"

using namespace Tungsten;
using namespace std::chrono_literals;
using Catch::Matchers::WithinAbs;

TEST_CASE("FixedStepClock: whole steps are returned, the rest carried over")
{
    FixedStepClock clock(10ms);
    REQUIRE(clock.advance(25ms) == 2);
    REQUIRE_THAT(clock.alpha(), WithinAbs(0.5, 1e-6));
    REQUIRE(clock.advance(4ms) == 0);
    REQUIRE_THAT(clock.alpha(), WithinAbs(0.9, 1e-6));
    REQUIRE(clock.advance(1ms) == 1);
    REQUIRE_THAT(clock.alpha(), WithinAbs(0.0, 1e-6));
}

TEST_CASE("FixedStepClock: from_rate divides a second into steps")
{
    const auto clock = FixedStepClock::from_rate(50);
    REQUIRE(clock.step() == 20ms);
    REQUIRE_THAT(clock.step_seconds(), WithinAbs(0.02, 1e-6));
}

TEST_CASE("FixedStepClock: a 60 Hz clock doesn't drift")
{
    auto clock = FixedStepClock::from_rate(60);
    uint32_t steps = 0;
    for (int i = 0; i < 1000; ++i)
        steps += clock.advance(1ms);
    REQUIRE(steps == 60);
}

TEST_CASE("FixedStepClock: time beyond max_steps is dropped")
{
    FixedStepClock clock(10ms, 3);
    REQUIRE(clock.advance(1s) == 3);
    REQUIRE_THAT(clock.alpha(), WithinAbs(0.0, 1e-6));
    REQUIRE(clock.advance(15ms) == 1);
}

TEST_CASE("FixedStepClock: negative elapsed time is ignored")
{
    FixedStepClock clock(10ms);
    REQUIRE(clock.advance(-5ms) == 0);
    REQUIRE(clock.advance(10ms) == 1);
}

TEST_CASE("FixedStepClock: reset discards accumulated time")
{
    FixedStepClock clock(10ms);
    clock.advance(9ms);
    clock.reset();
    REQUIRE(clock.advance(9ms) == 0);
}

TEST_CASE("FixedStepClock: the step must be positive")
{
    REQUIRE_THROWS_AS(FixedStepClock(0ms), TungstenException);
    REQUIRE_THROWS_AS(FixedStepClock::from_rate(0), TungstenException);
}
