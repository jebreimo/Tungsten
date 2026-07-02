//****************************************************************************
// Copyright © 2026 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2026-07-02.
//
// This file is distributed under the Zero-Clause BSD License.
// License text is included with the source distribution.
//****************************************************************************
#include "../../src/Tungsten/Neo/GenerationalPool.hpp"

#include <memory>
#include <catch2/catch_test_macros.hpp>

using Tungsten::GenerationalPool;
using Tungsten::ResourceRef;
using Tungsten::TungstenException;

TEST_CASE("GenerationalPool: insert then get returns the value")
{
    GenerationalPool<int> pool;
    auto ref = pool.insert(42);
    REQUIRE(pool.get(ref) == 42);
}

TEST_CASE("GenerationalPool: first live slot has generation 1")
{
    GenerationalPool<int> pool;
    auto ref = pool.insert(7);
    REQUIRE(ref.index == 0);
    REQUIRE(ref.generation == 1);
}

TEST_CASE("GenerationalPool: get is mutable")
{
    GenerationalPool<int> pool;
    auto ref = pool.insert(1);
    pool.get(ref) = 99;
    REQUIRE(pool.get(ref) == 99);
}

TEST_CASE("GenerationalPool: the null ref never resolves")
{
    GenerationalPool<int> pool;
    pool.insert(1); // makes index 0 a live slot at generation 1
    REQUIRE_THROWS_AS(pool.get(ResourceRef<int>{0, 0}), TungstenException);
}

TEST_CASE("GenerationalPool: out-of-range ref throws")
{
    GenerationalPool<int> pool;
    REQUIRE_THROWS_AS(pool.get(ResourceRef<int>{5, 1}), TungstenException);
}

TEST_CASE("GenerationalPool: erase invokes on_retire with the value")
{
    GenerationalPool<int> pool;
    auto ref = pool.insert(123);
    int retired = 0;
    pool.erase(ref, [&](int&& value) { retired = value; });
    REQUIRE(retired == 123);
}

TEST_CASE("GenerationalPool: a ref is stale after erase")
{
    GenerationalPool<int> pool;
    auto ref = pool.insert(5);
    pool.erase(ref, [](int&&) {});
    REQUIRE_THROWS_AS(pool.get(ref), TungstenException);
}

TEST_CASE("GenerationalPool: erase frees the slot for reuse with a new generation")
{
    GenerationalPool<int> pool;
    auto first = pool.insert(10);
    pool.erase(first, [](int&&) {});

    auto second = pool.insert(20);
    // Same slot index reused, but a bumped generation so the old ref stays dead.
    REQUIRE(second.index == first.index);
    REQUIRE(second.generation == first.generation + 1);
    REQUIRE(pool.get(second) == 20);
    REQUIRE_THROWS_AS(pool.get(first), TungstenException);
}

TEST_CASE("GenerationalPool: distinct slots get distinct indices")
{
    GenerationalPool<int> pool;
    auto a = pool.insert(1);
    auto b = pool.insert(2);
    REQUIRE(a.index != b.index);
    REQUIRE(pool.get(a) == 1);
    REQUIRE(pool.get(b) == 2);
}

TEST_CASE("GenerationalPool: holds move-only values")
{
    GenerationalPool<std::unique_ptr<int>> pool;
    auto ref = pool.insert(std::make_unique<int>(88));
    REQUIRE(*pool.get(ref) == 88);

    // on_retire can move the value out (the deferred-deletion contract): what it
    // takes ownership of is not destroyed by the erase.
    std::unique_ptr<int> rescued;
    pool.erase(ref, [&](std::unique_ptr<int>&& p) { rescued = std::move(p); });
    REQUIRE(rescued);
    REQUIRE(*rescued == 88);
}